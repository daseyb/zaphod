#pragma once
#include "../SimpleMath.h"
#include "../Rendering/Materials/Material.h"

struct BaseObject;

struct Intersection
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 normal;
	Material* material;
	BaseObject* hitObject;
};
