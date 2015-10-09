#include "BidirectionalPathTracer.h"
#include "../Scene.h"
#include "../BRDFs.h"
#include "../../Geometry/Intersection.h"
#include "../Materials/Material.h";

using namespace DirectX;
using namespace DirectX::SimpleMath;

#define RUSSIAN_ROULETTE 1.0f

BidirectionalPathTracer::Path BidirectionalPathTracer::MakePath(const DirectX::SimpleMath::Ray& _startRay, int _depth, std::default_random_engine& _rnd) const
{
	Path path;
	Ray ray = _startRay;

	std::uniform_real_distribution<float> dist(0, 1);

	float throughput = 1.0f;

	for (int i = 0; ;++i)
	{
		Intersection minIntersect;
		if (!m_Scene->Trace(ray, minIntersect))
		{
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
			float prob = throughput * RUSSIAN_ROULETTE;
			if (dist(_rnd) > prob) {
				break;
			}
			else {
				v.RelativeWeight = 1.0f/prob;
			}
		}

		auto sample = minIntersect.material->Sample(minIntersect, ray.direction, _rnd);
		v.Out = sample.Direction;
		v.BrdfWeight = sample.PDF;

		path.push_back(v);
		ray = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
		throughput *= v.BrdfWeight;
	}

	// Initialize additional values in vertices
	for (size_t i = 0; i < path.size() - 1; ++i)
	{
		path[i].TotalWeight = path[i].BrdfWeight * 
			std::abs(
				path[i].Normal.Dot(-path[i].Out) /
				Vector3::DistanceSquared(path[i].Pos, path[i+1].Pos)
			);
	}

	return path;
}

float BidirectionalPathTracer::G(PathVertex &v0, PathVertex &v1) const
{
	Vector3 w = (v1.Pos - v0.Pos);
	w.Normalize();
	return std::abs(v0.Normal.Dot(w)) * std::abs(v1.Normal.Dot(-w)) / Vector3::DistanceSquared(v0.Pos, v1.Pos);
}

Color BidirectionalPathTracer::EvalPath(Path& eye, int nEye, Path& light, int nLight) const {
	Color L(1, 1, 1, 1);
	int i;

	auto evalV = [](PathVertex v) {
		return v.Material->F(v.In, v.Out) *
			   v.Material->GetColor(v.Intersect) *
			   std::abs(v.Out.Dot(v.Normal)) / (v.BrdfWeight * v.RelativeWeight);
	};

	for (i = 0; i < nEye - 1; ++i)
	{
		L *= evalV(eye[i]);
	}

	Vector3 w = light[nLight - 1].Pos - eye[nEye - 1].Pos;

	Vector3 ww = -w;
	L *= eye[nEye - 1].Material->F(eye[nEye - 1].In, w) * 
		 eye[nEye - 1].Material->GetColor(eye[nEye - 1].Intersect) *
		 G(eye[nEye - 1], light[nLight - 1]) * light[nLight - 1].Material->F(ww, light[nLight - 1].In) / 
		 (eye[nEye - 1].RelativeWeight * light[nLight - 1].RelativeWeight);

	for (i = nLight - 2; i >= 0; --i)
	{
		L *= evalV(light[i]);
	}

	if (L.R() == 0. && L.G() == 0 && L.B() == 0)
	{
		return L;
	}

	if (!m_Scene->Test(eye[nEye - 1].Pos, light[nLight - 1].Pos))
	{
		return Color(0.);
	}

	return L;
}

Color BidirectionalPathTracer::Intersect(const Ray & _ray, int _depth, bool _isSecondary, std::default_random_engine & _rnd) const
{
	if (_depth == 0) {
		return Color(0, 0, 0);
	}

	BaseObject* sampledLight;
	float Le;
	Ray lightStart = m_Scene->SampleLight(_rnd, &sampledLight, Le);
	lightStart.direction = CosWeightedRandomHemisphereDirection2(lightStart.direction, _rnd);

	// Generate eye and light sub-paths
	auto lightPath = MakePath(lightStart, _depth, _rnd);
	auto eyePath = MakePath(_ray, _depth, _rnd);

	Color L(0, 0, 0, 0);
	int i, j;

	// Connect bidirectional path prefixes and evaluate throughput
	for (i = 1; i <= eyePath.size(); ++i)
	{
		for (j = 1; j <= lightPath.size(); ++j)
		{
			L += Le * EvalPath(eyePath, i, lightPath, j) / (i + j);
		}
	}

	return L;
}