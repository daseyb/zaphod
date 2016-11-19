#pragma once
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

class Texture {
public:
    virtual Color Sample(Vector2 uv) const = 0;
};