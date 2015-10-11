#pragma once
#include "Material.h"
struct SpecularMaterial : public Material
{
	DirectX::SimpleMath::Color DiffuseColor;
	float Kd, Ks, Roughness;

	SpecularMaterial(DirectX::SimpleMath::Color _color, float _kd, float _ks, float _roughness) : DiffuseColor(_color), Kd(_kd), Ks(_ks), Roughness(_roughness) {};

	inline virtual float F(DirectX::SimpleMath::Vector3 _in, DirectX::SimpleMath::Vector3 _out) const override {
		return std::abs(_in.Dot(_out));
	}

	inline virtual BRDFSample Sample(const Intersection& _intersect, DirectX::SimpleMath::Vector3 _view, std::default_random_engine& _rnd) const override {
		return BRDFDiffuse(_intersect.normal, _view, _rnd);
		//return BRDFPhong(_intersect.normal, _view, Kd, Ks, Roughness, _rnd);
	}

	inline virtual Color GetColor(const Intersection& _intersect) const override {
		return DiffuseColor;
	}

	inline virtual SpecularMaterial* Copy() { return new SpecularMaterial(*this); };

};

