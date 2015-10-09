#pragma once
#include "Material.h"

struct DiffuseMaterial : public Material
{
	DirectX::SimpleMath::Color DiffuseColor;

	DiffuseMaterial(DirectX::SimpleMath::Color _color) : DiffuseColor(_color) {};

	inline virtual float F(DirectX::SimpleMath::Vector3 _in, DirectX::SimpleMath::Vector3 _out) const override {
		return std::abs(_in.Dot(_out));
	}

	inline virtual BRDFSample Sample(const Intersection& _intersect, DirectX::SimpleMath::Vector3 _view, std::default_random_engine& _rnd) const override {
		return BRDFDiffuse(_intersect.normal, _view, _rnd);
	}

	inline virtual Color GetColor(const Intersection& _intersect) const override {
		return DiffuseColor;
	}

	inline virtual DiffuseMaterial* Copy() { return new DiffuseMaterial(*this); };
};

