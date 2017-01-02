#pragma once
#include "Material.h"
#include <Rendering/Textures/Texture.h>

struct EmissionMaterial : public Material {
  std::shared_ptr<Texture> Emittance;
  float strength;

  EmissionMaterial(std::shared_ptr<Texture> _color, float strength) : Emittance(_color), strength(strength) {};

  virtual float F(DirectX::SimpleMath::Vector3 _in, DirectX::SimpleMath::Vector3 _out, DirectX::SimpleMath::Vector3 _normal) const {
	return std::abs((_normal).Dot(_out));
  }

  virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    return BRDFDiffuse(_intersect.normal, _view, _rnd);
  }

  virtual bool IsLight() const override { return true; }

  virtual Color GetColor(const Intersection &_intersect) const override {
    return Emittance->Sample(_intersect.uv) * strength;
  }

  virtual EmissionMaterial *Copy() {
    return new EmissionMaterial(*this);
  };
};
