#pragma once
#include "SimpleMath.h"

struct Material
{
	DirectX::SimpleMath::Color DiffuseColor;
	DirectX::SimpleMath::Color SpecularColor;
	DirectX::SimpleMath::Color ReflectionColor;
	float DiffuseFactor;
	float SpecularFactor;
	float ReflectionFactor;
	float Transparency;
	float IOR;
};

