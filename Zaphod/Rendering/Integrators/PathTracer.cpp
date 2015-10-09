#include "PathTracer.h"
#include "../../Geometry/Intersection.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

//Intersect a ray with the scene (currently no optimization)
Color PathTracer::Intersect(const Ray & _ray, int _depth, bool _isSecondary, std::default_random_engine & _rnd) const
{
	if (_depth == 0) {
		return Color(0, 0, 0);
	}

	Intersection minIntersect;
	bool intersectFound = m_Scene->Trace(_ray, minIntersect);

	if (!intersectFound)
	{
		return Color(0, 0, 0);
	}

	if (minIntersect.material->IsLight()) {
		return minIntersect.material->GetColor(minIntersect);
	}

	Color reflected(0, 0, 0);

	auto sample = minIntersect.material->Sample(minIntersect, _ray.direction, _rnd);

	Ray diffuseRay = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
	reflected += Intersect(diffuseRay, _depth - 1, true, _rnd) * 
				 minIntersect.material->GetColor(minIntersect) * 
				 std::abs(_ray.direction.Dot(minIntersect.normal))/sample.PDF;

	return reflected;
}
