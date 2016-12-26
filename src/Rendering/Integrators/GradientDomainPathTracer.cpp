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
      bool valid = OffsetPath(basePath, offsetRays[i], baseLength, offsetPath, weight, jacobian);
      if(valid) result += weight * (basePathValue - EvaluatePath(offsetPath, baseLength) * jacobian);
    }

    result *= camWeight * 0.25f;
    result = { abs(result.x),abs(result.y),abs(result.z)};
  }
  return result;
}

#include <iostream>

bool GradientDomainPathTracer::OffsetPath(const Path& base, const Ray &startRay, int length, Path& offset, float& weight, float& jacobian) const {

  weight = 1;
  jacobian = 1;
  Ray currentRay = startRay;
  offset[0] = { { currentRay.position, currentRay.direction, { 0,0 }, nullptr, nullptr }, { currentRay.direction, 1, InteractionType::Diffuse }, PathVertex::Camera };

  int i = 1;
  for (; i < length; i++) {

    Intersection minIntersect;
    bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

    if (!intersectFound) {
      return false;
    }

    if (minIntersect.material->IsLight()) {
      offset[i] = { minIntersect,{ { 0, 0, 0 }, 1 },  PathVertex::Light };
      i++;
      break;
    }

    BRDFSample sample;
    auto halfVec = (-base[i - 1].sample.Direction + base[i].sample.Direction);
    halfVec.Normalize();
    sample.Direction = Vector3::Reflect(-offset[i - 1].sample.Direction, halfVec);
    sample.PDF = minIntersect.material->F(offset[i - 1].sample.Direction, sample.Direction);
    sample.Type = minIntersect.material->type;

    offset[i] = { minIntersect, sample, sample.Type == InteractionType::Diffuse ? PathVertex::Diffuse : PathVertex::Specular };

    bool thisIsDiffuse = base[i].type == PathVertex::Diffuse && offset[i].type == PathVertex::Diffuse;
    bool nextIsDiffuse = base[i + 1].type == PathVertex::Diffuse || base[i + 1].type == PathVertex::Light;

    bool canConnect = thisIsDiffuse && nextIsDiffuse;

    if (canConnect) {
      currentRay.direction = (base[i + 1].intersect.position - offset[i].intersect.position);
      currentRay.direction.Normalize();

      offset[i].sample.Direction = currentRay.direction;
      offset[i].sample.PDF = minIntersect.material->F(offset[i - 1].sample.Direction, offset[i].sample.Direction)/ minIntersect.material->F(base[i - 1].sample.Direction, base[i].sample.Direction);
      // Connection failed
      if (!m_Scene->Test(offset[i].intersect.position, base[i + 1].intersect.position)) return false;
      // Connection succeeded, copy the rest of the vertices

      i++;
      break;
    } else {
      currentRay = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
    }
  }

  for (; i < length; i++) {
    offset[i] = base[i];
  }

  return true;
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
      L = pathVertex.intersect.material->GetColor(pathVertex.intersect);
      break;
    }

    weight *= pathVertex.intersect.material->GetColor(pathVertex.intersect) * pathVertex.sample.PDF;
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
