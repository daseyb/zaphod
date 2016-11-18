#pragma once
#include <memory>
#include <SimpleMath.h>

using namespace DirectX;
using namespace DirectX::SimpleMath;

struct TextureData {
    std::shared_ptr<Color> pixels;

    int width, height;
};