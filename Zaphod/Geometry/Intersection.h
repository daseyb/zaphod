#pragma once
#include "../SimpleMath.h"

class BaseObject;
struct Material;

struct Intersection
{
	DirectX::SimpleMath::Vector3 position;
	DirectX::SimpleMath::Vector3 normal;
	Material* material;
	BaseObject* hitObject;
};
