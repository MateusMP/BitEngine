#pragma once

#include "Texture.h"

namespace BitEngine {

class Material {
public:
    virtual int getTextureCount() = 0;
    virtual RR<Texture> getTexture(int index) = 0;
};
}