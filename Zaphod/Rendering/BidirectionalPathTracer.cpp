#include "BidirectionalPathTracer.h"
#include "Scene.h"
#include "BRDFs.h"

using namespace DirectX;
using namespace DirectX::SimpleMath;

BidirectionalPathTracer::Path BidirectionalPathTracer::MakePath(DirectX::SimpleMath::Ray _startRay, Intersection _startIntersection, int _depth, std::default_random_engine& _rnd) const
{
	Path path;
	path.Vertices.push_back({ _startIntersection, _startRay.direction });
	Ray ray = _startRay;

	for (int i = 0; i < _depth; ++i)
	{
		Intersection minIntersect;
		bool intersectFound = m_Scene->Trace(ray, minIntersect);
		if (!intersectFound)
		{
			break;
		}

		auto sample = BRDFPhong(*m_Scene, _depth, minIntersect, ray.direction, _rnd);
		
		path.Vertices.push_back({ minIntersect, sample.Direction });
		ray = Ray(minIntersect.position + sample.Direction * 0.001f, sample.Direction);
	}

	return path;
}

BidirectionalPathTracer::Path BidirectionalPathTracer::ConnectPaths(Path beginning, int beginningIndex, int endIndex, Path end) const {
	std::vector<PathVertex> vertices{ std::begin(beginning.Vertices), std::begin(beginning.Vertices) + beginningIndex };
	for (int i = endIndex; i >= 0; --i) {
		vertices.push_back(end.Vertices[i]);
	}
	return{ vertices };
}

Color BidirectionalPathTracer::Intersect(const Ray & _ray, int _depth, bool _isSecondary, std::default_random_engine & _rnd) const
{
	if (_depth == 0) {
		return Color(0, 0, 0);
	}

	Intersection lightIntersection;
	Ray lightStart = m_Scene->SampleLight(_rnd, &lightIntersection.hitObject);
	lightIntersection.normal = lightStart.direction;
	lightIntersection.position = lightStart.position;
	lightIntersection.material = lightIntersection.hitObject->GetMaterial();
	lightStart.direction = CosWeightedRandomHemisphereDirection2(lightStart.direction, _rnd);

	int pathLength = _depth / 2;

	Path lightPath = MakePath(lightStart, lightIntersection, pathLength, _rnd);

	Intersection eyeIntersection;
	eyeIntersection.position = _ray.position;
	eyeIntersection.normal = _ray.direction;

	Path eyePath = MakePath(_ray, eyeIntersection, pathLength, _rnd);

	Color final = Color(0, 0, 0);
	float accumulatedPdf = 0;

	std::vector<Color> lightThroughputs(pathLength);
	std::vector<Color> eyeThroughputs(pathLength);


	for (int j = 0; j < pathLength; ++j) {
		PathVertex lightVertex = lightPath.Vertices[j];
		Color asl;

		for (int i = 1; i < pathLength; ++i) {
			PathVertex eyeVertex = eyePath.Vertices[i];
			
			//CST
			Ray shadowRay(eyeVertex.Intersection.position, lightVertex.Intersection.position - eyeVertex.Intersection.position);
			shadowRay.direction.Normalize();
			Intersection _;
			if (!m_Scene->Trace(shadowRay, _)) {
				continue;
			}

			Color ate;
			float weight;

			Color cst = weight * ate * asl;

			final += cst;

			Path connected = ConnectPaths(eyePath, i, j, lightPath);
			float pdf;
			Color pathThroughput = connected.CalculateThroughput(pdf);
			
			if (pdf < g_XMEpsilon.f[0]) {
				continue;
			}

			final = final * accumulatedPdf / (pdf + accumulatedPdf) + 
					pathThroughput * pdf / (pdf + accumulatedPdf);

			accumulatedPdf *= pdf;
		}
	}

	return final;
}

Color BidirectionalPathTracer::Path::CalculateThroughput(float & pdf) const
{
	return Color();
}
