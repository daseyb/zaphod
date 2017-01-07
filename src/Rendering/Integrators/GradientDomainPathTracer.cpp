#include "GradientDomainPathTracer.h"


#include "../Scene.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Color GradientDomainPathTracer::Sample(float x, float y, int w, int h, std::default_random_engine& _rnd) const {
  Color result = { 0, 0, 0, 0 };

  {
    float camWeight, weightOffset;
    Ray ray = m_Camera->GetRay(x, y, w, h, _rnd, camWeight);

    std::array<Ray, 4> offsetRays = {
      m_Camera->GetRay(x + 1, y, w, h, _rnd, weightOffset),
      m_Camera->GetRay(x, y + 1, w, h, _rnd, weightOffset),
      m_Camera->GetRay(x - 1, y, w, h, _rnd, weightOffset),
      m_Camera->GetRay(x, y - 1, w, h, _rnd, weightOffset),
    };

    int baseLength;
    auto basePath = TracePath(ray, 8, _rnd, baseLength);
    auto basePathValue = EvaluatePath(basePath, baseLength);

    for (int i = 0; i < 4; i++) {
      Path offsetPath;
      float weight, jacobian;
      int shiftLength;

      auto shiftResult = OffsetPath(basePath, offsetRays[i], baseLength, offsetPath, shiftLength, weight, jacobian);
      Color accum = { 0,0,0 };
      switch (shiftResult) {
        case ShiftResult::NotInvertible:
        {
          offsetPath = TracePath(offsetRays[i], 8, _rnd, shiftLength);
          accum = basePathValue - EvaluatePath(offsetPath, shiftLength);
          break;
        }
        case ShiftResult::Invertible:
        {
          float w_ij = 0.5f; //TODO: MIS
          accum = w_ij * (basePathValue - EvaluatePath(offsetPath, shiftLength) * jacobian);
          break;
        }
        case ShiftResult::NotSymmetric:
        {
          float w_ij = 1.0;
          accum = w_ij * (basePathValue - EvaluatePath(offsetPath, shiftLength) * jacobian);
        }
      }

	  float fwdFactor = (i > 1 ? -1 : 1);
      int gradientOffsetX = ( (i == 2 && int(x) != 0) ? -1 : 0);
      int gradientOffsetY = ( (i == 3 && int(y) != 0) ? -1 : 0);
      ds[i % 2].get()[m_Width * (int(y) + gradientOffsetY) + int(x) + gradientOffsetX] +=  -accum * fwdFactor;
      result = accum * fwdFactor;
    }
    base.get()[m_Width * int(y) + int(x)] += basePathValue;
    result = Color{ 0.5, 0.5, 0.5 } + Color{ result.x, result.y, result.z }*0.5f;
  }
  return result;
}

void GradientDomainPathTracer::Finalize(int totalSamples) const {
  Integrator::Finalize(totalSamples);
}


GradientDomainPathTracer::ShiftResult GradientDomainPathTracer::OffsetPath(const Path& base, const Ray &startRay, int length, Path& offset, int& shiftLength, float& weight, float& jacobian) const {
  weight = 1;
  jacobian = 1;
  shiftLength = 1;

  Ray currentRay = startRay;
  offset[0] = { { currentRay.position, currentRay.direction, { 0,0 }, nullptr, nullptr }, { currentRay.direction, 1, InteractionType::Diffuse }, PathVertex::Camera };

  auto result = ShiftResult::Invertible;

  int i = 1;
  for (; i < length; i++) {

    Intersection minIntersect;
    bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

    if (!intersectFound) {
      return ShiftResult::NotInvertible;
    }

    if (minIntersect.material->IsLight()) {
      offset[i] = { minIntersect,{ { 0, 0, 0 }, 1 },  PathVertex::Light };
      i++;
			return ShiftResult::NotInvertible;
    }


    BRDFSample sample;
    auto halfVec = (-base[i - 1].sample.Direction + base[i].sample.Direction);
    halfVec.Normalize();
    sample.Direction = Vector3::Reflect(-offset[i - 1].sample.Direction, halfVec);
    sample.PDF = minIntersect.material->F(offset[i - 1].sample.Direction, sample.Direction, minIntersect.normal);
    sample.Type = minIntersect.material->type;

		if (base[i].type == PathVertex::Specular) return ShiftResult::NotInvertible;

    offset[i] = { minIntersect, sample, sample.Type == InteractionType::Diffuse ? PathVertex::Diffuse : PathVertex::Specular };

    if (offset[i].type != base[i].type) result = ShiftResult::NotSymmetric;

    bool thisIsDiffuse = base[i].type == PathVertex::Diffuse && offset[i].type == PathVertex::Diffuse;
    bool nextIsDiffuse = base[i + 1].type == PathVertex::Diffuse || base[i + 1].type == PathVertex::Light;

    bool canConnect = thisIsDiffuse && nextIsDiffuse;

    if (canConnect) {
      currentRay.direction = (base[i + 1].intersect.position - offset[i].intersect.position);
      currentRay.direction.Normalize();

      offset[i].sample.Direction = currentRay.direction;
			offset[i].sample.PDF = minIntersect.material->F(offset[i - 1].sample.Direction, offset[i].sample.Direction, offset[i].intersect.normal);
      
      float squaredDistX = (base[i + 1].intersect.position - base[i].intersect.position).LengthSquared();
      float squaredDistY = (base[i + 1].intersect.position - offset[i].intersect.position).LengthSquared();
      
      float cosX = abs(base[i].sample.Direction.Dot(base[i].intersect.normal));
      float cosY = abs(offset[i].sample.Direction.Dot(offset[i].intersect.normal));

      jacobian *= (cosX * squaredDistX) / (0.000001f + cosY * squaredDistY);

      // Connection failed
      if (!m_Scene->Test(offset[i].intersect.position, base[i + 1].intersect.position)) {
        shiftLength = i;
        return ShiftResult::NotInvertible;
      }
      // Connection succeeded, copy the rest of the vertices

      i++;
      break;
    } else {
      // TODO: Jacobian
      currentRay = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
    }
  }

  for (; i < length; i++) {
    offset[i] = base[i];
  }
  shiftLength = length;
  return result;
}

GradientDomainPathTracer::Path GradientDomainPathTracer::TracePath(const Ray &_ray, int _depth, std::default_random_engine &_rnd, int& length) const {
  Path path;

  Ray currentRay = _ray;

  path[0] = { { currentRay.position, currentRay.direction,{ 0,0 }, nullptr, nullptr },{ currentRay.direction, 1, InteractionType::Diffuse }, PathVertex::Camera };

  int filledSamples = 1;

  for (int i = 1; i < _depth; i++) {
    Intersection minIntersect;
    bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

    if (!intersectFound) {
      break;
    }

    if (minIntersect.material->IsLight()) {
      path[i] = { minIntersect,{ { 0, 0, 0 }, 1 },  PathVertex::Light };
      filledSamples++;
      break;
    }

    auto sample =
      minIntersect.material->Sample(minIntersect, currentRay.direction, _rnd);

    currentRay = Ray(minIntersect.position + sample.Direction * 0.001f,
      sample.Direction);

    path[i] = { minIntersect, sample, sample.Type == InteractionType::Diffuse ? PathVertex::Diffuse : PathVertex::Specular };
    filledSamples++;
  }

  length = filledSamples;
  return path;
}

Color GradientDomainPathTracer::EvaluatePath(const Path& path, int length) const {

  if (path[length - 1].type != PathVertex::Light) return{ 0, 0, 0, 1 };

  Color weight = Color(1, 1, 1, 1);
  weight.A(0);
  Color L = Color(0, 0, 0, 1);

  for (int i = 1; i < length; i++) {
    auto& pathVertex = path[i];

    if (pathVertex.type == PathVertex::Light) {
      L = pathVertex.intersect.material->GetColor(pathVertex.intersect, pathVertex.sample.Type);
      break;
    }

    weight *= pathVertex.intersect.material->F(path[i-1].sample.Direction, path[i].sample.Direction, path[i].intersect.normal) 
			* pathVertex.intersect.material->GetColor(pathVertex.intersect, pathVertex.sample.Type) * std::abs(path[i].sample.Direction.Dot(path[i].intersect.normal)) / pathVertex.sample.PDF;
  }

  return L * weight;
}

// Intersect a ray with the scene (currently no optimization)
Color GradientDomainPathTracer::Intersect(const Ray &_ray, int _depth, bool _isSecondary,
  std::default_random_engine &_rnd) const {
  if (_depth == 0) {
    return Color(0, 0, 0);
  }

  int length;
  auto path = TracePath(_ray, _depth, _rnd, length);
  return EvaluatePath(path, length);
}
