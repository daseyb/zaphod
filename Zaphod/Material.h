#pragma once
#include "SimpleMath.h"

struct Material
{
	DirectX::SimpleMath::Color DiffuseColor;
	DirectX::SimpleMath::Color Emittance;
	float Roughness;
	Material() : Roughness(1), DiffuseColor(0, 0, 0), Emittance(0, 0, 0) { }
};

