#pragma once
#include "Integrator.h"

class PathTracer : Integrator
{
public:
	PathTracer(Scene* scene) : Integrator(scene) {}
	virtual DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary, std::default_random_engine& _rnd) const override;
};

