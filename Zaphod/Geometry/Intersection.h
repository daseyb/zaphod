#pragma once
#include "../SimpleMath.h"
#include "../Material.h"
#include "../Objects/BaseObject.h"

struct Intersection
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 normal;
	Material material;
  BaseObject* hitObject;
};
