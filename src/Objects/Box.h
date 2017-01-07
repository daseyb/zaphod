#pragma once
#include "RenderObject.h"
#include "../SimpleMath.h"

class Box : public RenderObject {
private:
  DirectX::BoundingBox m_Box;
  std::discrete_distribution<int> m_SampleDist;

  float m_SampleWeights[3];

public:
  Box(DirectX::SimpleMath::Vector3 _pos, DirectX::SimpleMath::Vector3 _extends, BaseObject* parent = nullptr);
  void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
  bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                 Intersection &_intersect)  override;
  float CalculateWeight() override;
  DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) override;
};
