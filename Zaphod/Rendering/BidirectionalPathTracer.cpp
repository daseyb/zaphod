#include "BidirectionalPathTracer.h"
#include "../Geometry/Intersection.h"
#include "Scene.h"
#include "BRDFs.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

Color BidirectionalPathTracer::Intersect(const Ray & _ray, int _depth, bool _isSecondary, std::default_random_engine & _rnd) const
{
	if (_depth == 0) {
		return Color(0, 0, 0);
	}

	BaseObject* sampledLight;
	Ray lightStart = m_Scene->SampleLight(_rnd, &sampledLight);



	Intersection minIntersect;
	bool intersectFound = m_Scene->Trace(_ray, minIntersect);

	if (!intersectFound)
	{
		return Color(0, 0, 0);
	}

	Color emittance = minIntersect.material.Emittance;
	Color reflected(0, 0, 0);

	auto sample = BRDFPhong(*m_Scene, _depth, minIntersect, _ray.direction, _rnd);

	Ray diffuseRay = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
	reflected += Intersect(diffuseRay, _depth - 1, true, _rnd) * sample.Color * sample.PDF;

	Color final = emittance + reflected;

	return final;
}
