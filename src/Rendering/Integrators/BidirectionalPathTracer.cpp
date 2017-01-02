#include "BidirectionalPathTracer.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../../Geometry/Intersection.h"
#include "../Materials/Material.h"
#include "../../Objects/RenderObject.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RUSSIAN_ROULETTE 0.9f


BidirectionalPathTracer::Path
BidirectionalPathTracer::MakePath(const DirectX::SimpleMath::Ray &_startRay,
                                  int _depth,
                                  std::default_random_engine &_rnd) const {
  Path path;
  path.reserve(_depth+1);
  Ray ray = _startRay;

  std::uniform_real_distribution<float> dist(0, 1);

  float throughput = 1.0f;

  for (int i = 0; i < _depth; ++i) {
    Intersection minIntersect;
    if (!m_Scene->Trace(ray, minIntersect)) {
      break;
    }

    PathVertex v;
    v.Intersect = minIntersect;
    v.Normal = minIntersect.normal;
    v.Pos = minIntersect.position;
    v.Material = minIntersect.material;
    v.In = ray.direction;

    v.RelativeWeight = 1;

    if (i > 2) {
      if (dist(_rnd) > RUSSIAN_ROULETTE) {
        break;
      } else {
        v.RelativeWeight = 1.0f / RUSSIAN_ROULETTE;
      }
    }

    auto sample =
        minIntersect.material->Sample(minIntersect, ray.direction, _rnd);
    v.Out = sample.Direction;
    v.BrdfWeight = sample.PDF;

    path.push_back(v);
    ray = {minIntersect.position + sample.Direction * 0.001f, sample.Direction};
  }

  return path;
}

float BidirectionalPathTracer::G(const PathVertex &v0,
                                 const PathVertex &v1) const {
  Vector3 w = (v1.Pos - v0.Pos);
  w.Normalize();
  return std::abs(v0.Normal.Dot(w)) * std::abs(v1.Normal.Dot(-w)) /
         Vector3::DistanceSquared(v0.Pos, v1.Pos);
}

Color BidirectionalPathTracer::EvalPath(const Path &eye, int nEye,
                                        const Path &light, int nLight) const {

  if (nEye == 0 || nLight == 0) return{ 0, 0, 0 };
  Color L(1, 1, 1, 1);

  const static auto evalV = [](const PathVertex &v) {
    return v.Material->F(v.In, v.Out, v.Normal) * v.Material->GetColor(v.Intersect) *
           std::abs(v.Out.Dot(v.Normal)) / (v.BrdfWeight * v.RelativeWeight);
  };

  for (int i = 0; i < nEye - 1; ++i) {
    L *= evalV(eye[i]);
  }

  Vector3 w = light[nLight - 1].Pos - eye[nEye - 1].Pos;
  w.Normalize();
  Vector3 ww = -w;

  L *= eye[nEye - 1].Material->F(eye[nEye - 1].In, w, eye[nEye-1].Normal) *
       eye[nEye - 1].Material->GetColor(eye[nEye - 1].Intersect) *
       G(eye[nEye - 1], light[nLight - 1]) *
       light[nLight - 1].Material->F(ww, light[nLight - 1].In, light[nLight - 1].Normal) /
       (eye[nEye - 1].RelativeWeight * light[nLight - 1].RelativeWeight);

  for (int i = nLight - 2; i >= 0; --i) {
    L *= evalV(light[i]);
  }

  if (L.R() == 0.0f && L.G() == 0.0f && L.B() == 0.0f) {
    return L;
  }

  if (!m_Scene->Test(eye[nEye - 1].Pos, light[nLight - 1].Pos)) {
    return Color(0.0f, 0.0f, 0.0f);
  }

  return L;
}

Color BidirectionalPathTracer::IlluminatePoint(
    Vector3 pos, Vector3 normal, std::default_random_engine &_rnd) const {
  RenderObject *sampledLight;
  float Le;
  Ray lightStart = m_Scene->SampleLight(_rnd, &sampledLight, Le);
  lightStart.direction =
      CosWeightedRandomHemisphereDirection2(lightStart.direction, _rnd);

  Color L = sampledLight->GetMaterial()->GetColor(Intersection());
  L *= G(PathVertex{pos, normal},
         PathVertex{lightStart.position, lightStart.direction}) *
       Le * XM_PI;

  if (L.R() == 0.0f && L.G() == 0.0f && L.B() == 0.0f) {
    return L;
  }

  if (!m_Scene->Test(pos, lightStart.position)) {
    return Color(0.0f, 0.0f, 0.0f);
  }
  return L;
}

Color BidirectionalPathTracer::Intersect(const Ray &_ray, int _depth, bool _isSecondary, std::default_random_engine &_rnd) const {
  if (_depth == 0) {
    return Color(0, 0, 0);
  }

  RenderObject *sampledLight;
  float Le;
  Ray lightStart = m_Scene->SampleLight(_rnd, &sampledLight, Le);
  lightStart.direction =
      CosWeightedRandomHemisphereDirection2(lightStart.direction, _rnd);

  // Generate eye and light sub-paths
  auto lightPath = MakePath(lightStart, _depth, _rnd);
  auto eyePath = MakePath(_ray, _depth, _rnd);

  Color L(0, 0, 0, 0);
  size_t i, j;

  L = EvalPath(eyePath, eyePath.size(), lightPath, lightPath.size()) * Color(1,1,1);

#if 0
  // Connect bidirectional path prefixes and evaluate throughput
  Color directWt(1.0f, 1.0f, 1.0f);
  for (i = 1; i <= eyePath.size(); ++i) {
    /*// Handle direct lighting for bidirectional integrator
    directWt *= 1.0f/eyePath[i - 1].RelativeWeight;
    Color col = eyePath[i - 1].Material->GetColor(eyePath[i - 1].Intersect);
    L += directWt * col * IlluminatePoint(eyePath[i - 1].Pos, eyePath[i -
    1].Normal, _rnd) / i;
    directWt *= col * std::abs(eyePath[i - 1].Out.Dot(eyePath[i - 1].Normal)) /
    eyePath[i - 1].BrdfWeight;*/

    for (j = 1; j <= lightPath.size(); ++j) {
      L += EvalPath(eyePath, i, lightPath, j) / (float)(i + j);
    }
  }
#endif

  return L * Le;
}