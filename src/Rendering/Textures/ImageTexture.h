#pragma once

#include "Texture.h"
#include "TextureData.h"

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
    std::shared_ptr<const TextureData> texData;

public:
    ImageTexture(std::shared_ptr<const TextureData> data) : texData(data) {}

    TextureWrapMode wrapMode;
    TextureFilterMode filterMode;

    virtual Color Sample(Vector2 uv) const override {

        if (wrapMode == TextureWrapMode::Wrap) {
            uv = { fmodf(uv.x, 1.0), fmodf(uv.y, 1.0) };
        }

        uv.Clamp(Vector2(0, 0), Vector2(1, 1));

        auto pixel = uv * Vector2{ (float)texData->width-1, (float)texData->height-1 };
        auto id = int(pixel.x) + int(pixel.y) * texData->width;
        return texData->pixels.get()[id];
    }
};