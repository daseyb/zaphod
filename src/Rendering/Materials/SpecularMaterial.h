#pragma once
#include "Material.h"
#include "../Textures/Texture.h"

struct SpecularMaterial : public Material {
  std::shared_ptr<Texture> DiffuseColor;
	std::shared_ptr<Texture> SpecularColor;
	float Kd, Ks, Kt, Roughness;

  SpecularMaterial(std::shared_ptr<Texture> _color, float _kd, float _ks, float _kt,
                   float _roughness)
      : DiffuseColor(_color), SpecularColor(_color), Kd(_kd), Ks(_ks), Kt(_kt), Roughness(_roughness){
    type = InteractionType::Specular;
  };

	SpecularMaterial(std::shared_ptr<Texture> _color, std::shared_ptr<Texture> _spec, float _kd, float _ks, float _kt,
		float _roughness)
		: DiffuseColor(_color), SpecularColor(_spec), Kd(_kd), Ks(_ks), Kt(_kt), Roughness(_roughness) {
		type = InteractionType::Specular;
	};


	virtual float F(DirectX::SimpleMath::Vector3 _in, DirectX::SimpleMath::Vector3 _out, DirectX::SimpleMath::Vector3 _normal) const {
		
		float inside = sign(_in.Dot(_normal));
		auto refl = Vector3::Reflect(_in, -inside * _normal);
		float dot = std::abs(refl.Dot(_out));

		float specBrdf = 0;
		if (Roughness == 0) specBrdf = dot > 0.99 ? 1 : 0;
		else specBrdf = pow(dot, 1.0f / Roughness);

		const float ior = 1.5f;
		float n1 = inside < 0 ? 1.0 / ior : ior;
		float n2 = 1.0 / n1;

		auto refr = Vector3::Refract(_in, -inside * _normal, n1);
		if (refr.LengthSquared() < 0.5f) {
			auto refr = Vector3::Reflect(_in, -inside * _normal);
		}

		dot = std::abs(refr.Dot(_out));
		float transBrdf = 0;
		if (Roughness == 0) transBrdf = dot > 0.99 ? 1 : 0;
		else transBrdf = pow(dot, 1.0f / Roughness);

		float diffBrdf = 1.0f;

		float fresnel = FresnelSchlick(_in, _normal, (n1 - n2) / (n1 + n2));

		float ks = Ks;// *fresnel;
		float kd = Kd;// * (1.0f - fresnel);
		float kt = Kt;// * (1.0f - fresnel);

		float total = kd + ks + kt;
		
		kd = kd/total;
		ks = ks/total;
		kt = kt/total;
		
		float gs = std::abs(_out.Dot(_normal));

		specBrdf /= gs;
		transBrdf /= gs;

		return diffBrdf * kd + specBrdf * ks + transBrdf * kt;
	}

  virtual BRDFSample
  Sample(const Intersection &_intersect, DirectX::SimpleMath::Vector3 _view,
         std::default_random_engine &_rnd) const override {
    auto sample = BRDFPhong(_intersect.normal, _view, Kd, Ks, Kt, Roughness, _rnd);
    return sample;
  }

  virtual Color GetColor(const Intersection &_intersect, InteractionType type) const override {
    return type == InteractionType::Diffuse ? DiffuseColor->Sample(_intersect.uv) : SpecularColor->Sample(_intersect.uv);
  }

  virtual SpecularMaterial *Copy() {
    return new SpecularMaterial(*this);
  };
};
