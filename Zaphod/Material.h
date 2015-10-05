#pragma once
#include "SimpleMath.h"

struct Material {
	DirectX::SimpleMath::Color DiffuseColor;
	DirectX::SimpleMath::Color Emittance;
	float Kd;
	float Ks;
	float Roughness;
	Material() : Kd(1), Ks(0), Roughness(1), DiffuseColor(0, 0, 0), Emittance(0, 0, 0) { }
};

