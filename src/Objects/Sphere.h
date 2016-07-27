#pragma once
#include "baseobject.h"
#include "../SimpleMath.h"

class Sphere : public BaseObject {
private:
  DirectX::BoundingSphere m_Sphere;

public:
  Sphere(DirectX::SimpleMath::Vector3 _position, float _radius);
  ~Sphere(void);
  void SetRadius(float _radius);
  void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
  bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                 Intersection &_intersect) const override;
  float CalculateWeight() override;
  DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) const override;
};