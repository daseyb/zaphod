#pragma once
#include "Integrator.h"
#include "../Geometry/Intersection.h"

class BidirectionalPathTracer : public Integrator {
private:

	struct PathVertex {
		Intersection Intersection;
		DirectX::SimpleMath::Vector3 Outgoing;
	};

	struct Path {
		std::vector<PathVertex> Vertices;
		DirectX::SimpleMath::Color CalculateThroughput(float& pdf) const;
	};

	Path MakePath(DirectX::SimpleMath::Ray _startRay, Intersection _startIntersection, int _depth, std::default_random_engine& _rnd) const;
	Path ConnectPaths(Path beginning, int beginningIndex, int endIndex, Path end) const;

public:
	BidirectionalPathTracer(Scene* scene) : Integrator(scene) {}
	virtual DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary, std::default_random_engine& _rnd) const override;
};

