#pragma once
#include "Integrator.h"
#include "../../Geometry/Intersection.h"

struct Material;

class BidirectionalPathTracer : public Integrator {
private:

	struct PathVertex {
		DirectX::SimpleMath::Vector3 Pos;
		DirectX::SimpleMath::Vector3 Normal;
		DirectX::SimpleMath::Vector3 In;
		DirectX::SimpleMath::Vector3 Out;

		Intersection Intersect;
		Material* Material;
		BaseObject* Prim;
		float BrdfWeight;
		float TotalWeight;
		float RelativeWeight;
	};

	typedef std::vector<PathVertex> Path;

	Path MakePath(const DirectX::SimpleMath::Ray& _startRay, int _depth, std::default_random_engine& _rnd) const;
	float G(PathVertex & v0, PathVertex & v1) const;
	DirectX::SimpleMath::Color EvalPath(Path& eye, int nEye, Path& light, int nLight) const;

public:
	BidirectionalPathTracer(Scene* scene) : Integrator(scene) {}
	virtual DirectX::SimpleMath::Color Intersect(const DirectX::SimpleMath::Ray& _ray, int _depth, bool _isSecondary, std::default_random_engine& _rnd) const override;
};

