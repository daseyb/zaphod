#pragma once
#include "Material.h"

struct EmissionMaterial : public Material {
  DirectX::SimpleMath::Color Emittance;

  EmissionMaterial(DirectX::SimpleMath::Color _color) : Emittance(_color){};

  virtual float F(DirectX::SimpleMath::Vector3 _in,
                         DirectX::SimpleMath::Vector3 _out) const override {
    return XM_PI;
  }

  virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    return BRDFDiffuse(_intersect.normal, _view, _rnd);
  }

  virtual bool IsLight() const override { return true; }

  virtual Color GetColor(const Intersection &_intersect) const override {
    return Emittance;
  }

  virtual EmissionMaterial *Copy() {
    return new EmissionMaterial(*this);
  };
};
