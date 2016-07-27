#pragma once
#include "../../SimpleMath.h"
#include "../BRDFs.h"

struct Intersection;

struct Material {
  inline virtual float Material::F(DirectX::SimpleMath::Vector3 _in,
                                   DirectX::SimpleMath::Vector3 _out) const {
    return 1.0f;
  }
  inline virtual BRDFSample
  Material::Sample(const Intersection &_intersect,
                   DirectX::SimpleMath::Vector3 _view,
                   std::default_random_engine &_rnd) const {
    return BRDFSample();
  }
  inline virtual bool Material::IsLight() const { return false; }
  inline virtual DirectX::SimpleMath::Color
  Material::GetColor(const Intersection &_intersect) const {
    return Color(0.0f, 0.0f, 0.0f);
  }
  inline virtual Material *Copy() { return new Material(*this); };
};