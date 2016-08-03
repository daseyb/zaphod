#pragma once
#include "Material.h"
struct SpecularMaterial : public Material {
  DirectX::SimpleMath::Color DiffuseColor;
  float Kd, Ks, Kt, Roughness;

  SpecularMaterial(DirectX::SimpleMath::Color _color, float _kd, float _ks, float _kt,
                   float _roughness)
      : DiffuseColor(_color), Kd(_kd), Ks(_ks), Kt(_kt), Roughness(_roughness){};

  virtual float F(DirectX::SimpleMath::Vector3 _in,
                         DirectX::SimpleMath::Vector3 _out) const override {
    return std::abs(_in.Dot(_out));
  }

  virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    return BRDFPhong(_intersect.normal, _view, Kd, Ks, Kt, Roughness, _rnd);
  }

  virtual Color GetColor(const Intersection &_intersect) const override {
    return DiffuseColor;
  }

  virtual SpecularMaterial *Copy() {
    return new SpecularMaterial(*this);
  };
};
