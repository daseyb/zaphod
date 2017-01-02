#include "PathTracer.h"
#include "../../Geometry/Intersection.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"
#include "../../Objects/BaseObject.h"
#include "../../Objects/Sphere.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RUSSIAN_ROULETTE 1.0f

// Intersect a ray with the scene (currently no optimization)
Color PathTracer::Intersect(const Ray &_ray, int _depth, bool _isSecondary,
                            std::default_random_engine &_rnd) const {
  if (_depth == 0) {
    return Color(0, 0, 0);
  }

  Color weight = Color(1, 1, 1);
  weight.A(0);
  Color L = Color(0, 0, 0, 0);

  Ray currentRay = _ray;
  std::uniform_real_distribution<float> dist(0, 1);

  for (int i = 0; i < _depth; i++) {
    Intersection minIntersect;
    bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

    if (!intersectFound) {
      break;
    }

    if (minIntersect.material->IsLight()) {
        L += minIntersect.material->GetColor(minIntersect) * weight;
        break;
    }

    float rr_weight = 1;

    if (i > 4) {
      if (dist(_rnd) > RUSSIAN_ROULETTE) {
        break;
      } else {
        rr_weight = 1.0f / RUSSIAN_ROULETTE;
      }
    }

    auto sample =
        minIntersect.material->Sample(minIntersect, currentRay.direction, _rnd);

		weight *= minIntersect.material->F(currentRay.direction, sample.Direction, minIntersect.normal) * minIntersect.material->GetColor(minIntersect) * std::abs(sample.Direction.Dot(minIntersect.normal)) / sample.PDF * rr_weight;

    currentRay = Ray(minIntersect.position + sample.Direction * 0.001f,
                     sample.Direction);

    if (weight.ToVector3().LengthSquared() < 0.001) {
        break;
    }
  }

	auto vec = L.ToVector3();
	//vec.Clamp({ 0, 0, 0 }, { 100,100,100 });

	return Color(vec);
}
