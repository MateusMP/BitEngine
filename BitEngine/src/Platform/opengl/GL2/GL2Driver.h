#pragma once

#include <BitEngine/Core/Graphics/Color.h>

#include "Platform/opengl/FrameBuffer.h"

#include "Platform/video/BlendMaterial.h"

namespace BitEngine {

class GLVideoDriver {
public:
    static void clearBuffer(RenderBuffer* buffer, BufferClearBitMask mask);

    static void clearBufferColor(RenderBuffer* buffer, const ColorRGBA& color);

    static void setViewPort(int x, int y, int width, int height);

    static void configure(const BlendMaterial* material);
};
}