#include "PathTracer.h"
#include "../../Geometry/Intersection.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"
#include "../../Objects/BaseObject.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RUSSIAN_ROULETTE 0.9f

//Intersect a ray with the scene (currently no optimization)
Color PathTracer::Intersect(const Ray & _ray, int _depth, bool _isSecondary, std::default_random_engine & _rnd) const
{
	if (_depth == 0) {
		return Color(0, 0, 0);
	}

	Color L = Color(1, 1, 1);

	Ray currentRay = _ray;
	std::uniform_real_distribution<float> dist(0, 1);

	for (int i = 0; i < _depth; i++) {
		Intersection minIntersect;
		bool intersectFound = m_Scene->Trace(currentRay, minIntersect);

		if (!intersectFound)
		{
			L *= 0;
			break;
		}

		if (minIntersect.material->IsLight()) {
			L *= minIntersect.material->GetColor(minIntersect);
			break;
		}

		float weight = 1;

		if (i > 2) {
			if (dist(_rnd) > RUSSIAN_ROULETTE) {
				break;
			}
			else {
				weight = 1.0f / RUSSIAN_ROULETTE;
			}
		}

		auto sample = minIntersect.material->Sample(minIntersect, _ray.direction, _rnd);

		Ray diffuseRay = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
		
		L *= minIntersect.material->GetColor(minIntersect) *
			 std::abs((diffuseRay.direction).Dot(minIntersect.normal)) / sample.PDF * weight;

		currentRay = diffuseRay;
	}

	return L;
}
