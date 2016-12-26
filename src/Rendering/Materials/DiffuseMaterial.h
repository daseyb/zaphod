#pragma once
#include "Material.h"
#include "../Textures/Texture.h"

struct DiffuseMaterial : public Material {

  std::shared_ptr<Texture> DiffuseColor;
  DiffuseMaterial(std::shared_ptr<Texture> _color) : DiffuseColor(_color){
    type = InteractionType::Diffuse;
  };

  inline virtual float F(DirectX::SimpleMath::Vector3 _in,
                         DirectX::SimpleMath::Vector3 _out) const override {
    return std::abs((-_in).Dot(_out));
  }

  inline virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    return BRDFDiffuse(_intersect.normal, _view, _rnd);
  }

  inline virtual Color GetColor(const Intersection &_intersect) const override {
    return DiffuseColor->Sample(_intersect.uv);
  }

  inline virtual DiffuseMaterial *Copy() { return new DiffuseMaterial(*this); };
};
