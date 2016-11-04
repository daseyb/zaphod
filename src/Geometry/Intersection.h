#pragma once
#include "../SimpleMath.h"

class RenderObject;
struct Material;

struct Intersection {
  DirectX::SimpleMath::Vector3 position;
  DirectX::SimpleMath::Vector3 normal;
  Material *material;
  RenderObject *hitObject;
};
