#pragma once
#include "Material.h"
#include "../Textures/Texture.h"

struct SpecularMaterial : public Material {
  std::shared_ptr<Texture> DiffuseColor;
  float Kd, Ks, Kt, Roughness;

  SpecularMaterial(std::shared_ptr<Texture> _color, float _kd, float _ks, float _kt,
                   float _roughness)
      : DiffuseColor(_color), Kd(_kd), Ks(_ks), Kt(_kt), Roughness(_roughness){
    type = InteractionType::Specular;
  };

  virtual float F(DirectX::SimpleMath::Vector3 _in,
                         DirectX::SimpleMath::Vector3 _out) const override {
    float dot = std::abs((-_in).Dot(_out));

    if (Roughness == 0) return dot;

    return pow(dot, 1.0f / Roughness);
  }

  virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    auto sample = BRDFPhong(_intersect.normal, _view, Kd, Ks, Kt, Roughness, _rnd);
    sample.Type = type;
    return sample;
  }

  virtual Color GetColor(const Intersection &_intersect) const override {
    return DiffuseColor->Sample(_intersect.uv);
  }

  virtual SpecularMaterial *Copy() {
    return new SpecularMaterial(*this);
  };
};
