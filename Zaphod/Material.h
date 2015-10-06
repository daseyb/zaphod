#pragma once
#include "SimpleMath.h"

struct Material {
	DirectX::SimpleMath::Color DiffuseColor;
	DirectX::SimpleMath::Color Emittance;
	float Kd;
	float Ks;
	float Roughness;
	Material() : Kd(1), Ks(0), Roughness(1), DiffuseColor(0, 0, 0), Emittance(0, 0, 0) { }
	Material(DirectX::SimpleMath::Color diffuseColor, DirectX::SimpleMath::Color emittance, float kd, float ks, float roughness) : 
		Kd(kd), Ks(ks), Roughness(roughness), DiffuseColor(diffuseColor), Emittance(emittance) { }
};

