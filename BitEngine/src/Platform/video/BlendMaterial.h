#pragma once

#include <cstring>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Graphics/Texture.h"
#include "BitEngine/Core/Graphics/Material.h"
#include "BitEngine/Core/Resources/PropertyHolder.h"
#include "Platform/video/VideoRenderer.h"

namespace BitEngine {

class BE_API BlendMaterial : public Material {
public:
    BlendMaterial()
        : blendEquation(BlendEquation::ADD)
    {
        memset(states, 0, sizeof(states));
    }

    void setState(RenderConfig config, u8 mode)
    {
        states[(u8)config] = mode;
    }
    void setBlendMode(BlendFunc src, BlendFunc dst)
    {
        srcColorBlendMode = src;
        dstColorBlendMode = dst;
    }
    void setBlendModeSeparate(BlendFunc srcColor, BlendFunc dstColor, BlendFunc srcAlpha, BlendFunc dstAlpha)
    {
        srcColorBlendMode = srcColor;
        dstColorBlendMode = dstColor;
        srcAlphaBlendMode = srcAlpha;
        dstAlphaBlendMode = dstAlpha;
    }
    void setBlendEquation(BlendEquation eq)
    {
        blendEquation = eq;
    }

    u8 getState(RenderConfig config) const
    {
        return states[(u8)config];
    }

    virtual int getTextureCount() override { return 1; }
    virtual RR<Texture> getTexture(int index) override { return RR<Texture>(); }

    u8 states[(u8)RenderConfig::TOTAL_RENDER_CONFIGS];
    BlendFunc srcColorBlendMode;
    BlendFunc srcAlphaBlendMode;
    BlendFunc dstColorBlendMode;
    BlendFunc dstAlphaBlendMode;
    BlendEquation blendEquation;
};
}
