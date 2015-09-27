#pragma once
#include "baseobject.h"
#include "../SimpleMath.h"

class Box :
	public BaseObject
{
private:
	DirectX::BoundingBox m_Box;
  std::piecewise_constant_distribution<double> m_SampleDist;

  float m_SampleInterval[4];
  float m_SampleWeights[3];

public:
	Box(DirectX::SimpleMath::Vector3 _pos, float _extendX, float _extendY, float _extendZ);
	~Box(void);
	void SetExtendX(float _x);
	void SetExtendY(float _y);
	void SetExtendZ(float _z);
	void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
	bool Intersect(const DirectX::SimpleMath::Ray& _ray, Intersection& _intersect) const override;
  float CalculateWeight() override;
  DirectX::SimpleMath::Ray Sample(std::default_random_engine rnd) const override;
};

