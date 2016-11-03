#pragma once
#include "baseobject.h"
#include "../SimpleMath.h"

class AlembicCache : public BaseObject {
private:

public:
  AlembicCache(DirectX::SimpleMath::Vector3 _pos, DirectX::SimpleMath::Vector3 _extends);
  ~AlembicCache(void);
  void SetPosition(DirectX::SimpleMath::Vector3 _pos) override;
  bool Intersect(const DirectX::SimpleMath::Ray &_ray,
                 Intersection &_intersect) const override;
  float CalculateWeight() override;
  DirectX::SimpleMath::Ray
  Sample(std::default_random_engine &rnd) const override;
};
