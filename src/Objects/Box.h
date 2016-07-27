#pragma once
#include "baseobject.h"
#include "../SimpleMath.h"

class Box : public BaseObject {
private:
  DirectX::BoundingBox m_Box;
  std::discrete_distribution<int> m_SampleDist;

  float m_SampleWeights[3];

public:
  Box(DirectX::SimpleMath::Vector3 _pos, DirectX::SimpleMath::Vector3 _extends);
  ~Box(void);
  void SetExtendX(float _x);
  void SetExtendY(float _y);
  void SetExtendZ(float _z);
  void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
  bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                 Intersection &_intersect) const override;
  float CalculateWeight() override;
  DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) const override;
};
