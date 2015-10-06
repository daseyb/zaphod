#pragma once
#include "Integrator.h"
class BidirectionalPathTracer : public Integrator
{
	struct PathVertex
	{
		
	};


	struct Path {
		std::vector<BRDFSample> Vertices;
	};

public:
	BidirectionalPathTracer(Scene* scene) : Integrator(scene) {}
	virtual DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary, std::default_random_engine& _rnd) const override;
};

