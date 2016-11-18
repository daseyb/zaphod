#pragma once

#include "Texture.h"
#include <memory>


enum class TextureWrapMode {
    Clamp,
    Wrap
};

enum class TextureFilterMode {
    Point,
    Bilinear,
    Trilinear
};

class ImageTexture : public Texture {
private:
    Vector2 size;
    std::unique_ptr<Color[]> pixels;

public:
    TextureWrapMode wrapMode;
    TextureFilterMode filterMode;

    virtual Color Sample(Vector2 uv) const override {
        switch (wrapMode) {
            case TextureWrapMode::Clamp: uv.Clamp(Vector2(0, 0), Vector2(1, 1)); break;
            case TextureWrapMode::Wrap: uv = { fmodf(uv.x, 1.0), fmodf(uv.y, 1.0) }; break;
        }

        auto pixel = uv * size;
        auto id = int(pixel.x) + int(pixel.y) * int(size.x);
        return pixels[id];
    }
};