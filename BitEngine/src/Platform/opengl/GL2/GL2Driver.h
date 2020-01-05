#pragma once

#include "Platform/opengl/FrameBuffer.h"

namespace BitEngine {

class GLVideoDriver {
public:

    static void clearBuffer(RenderBuffer* buffer, BufferClearBitMask mask);

    static void clearBufferColor(RenderBuffer* buffer, const ColorRGBA& color);

    static void setViewPort(int x, int y, int width, int height);

    static void configure(const Material* material);

};

}