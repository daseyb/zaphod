#include "PathTracer.h"
#include "../../Geometry/Intersection.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../Materials/Material.h"
#include "../../Objects/BaseObject.h"

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
	Color indirect = Intersect(diffuseRay, _depth - 1, true, _rnd) * 
				 minIntersect.material->GetColor(minIntersect) * 
				 std::abs((diffuseRay.direction).Dot(minIntersect.normal))/sample.PDF;

	BaseObject* sampledLight;
	float Le;
	Ray lightStart = m_Scene->SampleLight(_rnd, &sampledLight, Le);
	lightStart.direction = CosWeightedRandomHemisphereDirection2(lightStart.direction, _rnd);

	Vector3 v0 = diffuseRay.position;
	Vector3 v1 = lightStart.position;

	Vector3 w = v1 - v0;
	Color direct = sampledLight->GetMaterial()->GetColor(Intersection());
	float g = std::abs(minIntersect.normal.Dot(w)) * std::abs(lightStart.direction.Dot(-w)) / Vector3::DistanceSquared(v0, v1) * Le;

	direct *= g;

	if (!m_Scene->Test(v0, v1))
	{
		direct = Color(0.f);
	}

	float directW = g / (sample.PDF + g);

	return indirect * (1.0f - directW) + direct * directW;
}
