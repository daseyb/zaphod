#pragma once

#include "Texture.h"

class ConstantColor : public Texture {
private:
    Color color;
public:
    ConstantColor(Color color) : color(color) { }
    virtual Color Sample(Vector2 uv) const override {
        return color;
    }
};