#pragma once
#include "Material.h"
#include "../Textures/Texture.h"

struct TransparentMaterial : public Material {

  std::shared_ptr<Texture> Opacity;
  Material* ChildMat;

  TransparentMaterial(std::shared_ptr<Texture> _opacity, Material* _childMat) : Opacity(_opacity), ChildMat(_childMat) {
    type = ChildMat->type;
  };

  virtual float F(DirectX::SimpleMath::Vector3 _in, DirectX::SimpleMath::Vector3 _out, DirectX::SimpleMath::Vector3 _normal) const {
    return ChildMat->F(_in, _out, _normal);
  }

  inline virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    auto opacity = Opacity->Sample(_intersect.uv);

    auto prob = opacity.R() + opacity.G() + opacity.B();

    prob *= 1.0f / 3.0f;

    std::uniform_real_distribution<float> dist(0, 1);

    auto fac = dist(_rnd);

    if (fac < prob) {
        return{ _view, 1.0, InteractionType::Passthrough };
    } else {
        return ChildMat->Sample(_intersect, _view, _rnd);
    }

  }

	virtual DirectX::SimpleMath::Color GetColor(const Intersection &_intersect, InteractionType type) const override {
    return type == InteractionType::Passthrough ? Opacity->Sample(_intersect.uv) : ChildMat->GetColor(_intersect, type);
  }

  inline virtual TransparentMaterial *Copy() { return new TransparentMaterial(*this); };
};
