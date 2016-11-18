#pragma once
#include "../SimpleMath.h"

class RenderObject;
struct Material;

struct Intersection {
  DirectX::SimpleMath::Vector3 position;
  DirectX::SimpleMath::Vector3 normal;
  DirectX::SimpleMath::Vector2 uv;
  Material *material;
  RenderObject *hitObject;
};
