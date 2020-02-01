
#include "BitEngine/Core/Memory.h"

#include "Platform/opengl/GL2/OpenGL2.h"
#include "Platform/opengl/GL2/GL2Impl.h"
#include "Platform/opengl/GL2/GL2ShaderManager.h"
#include "Platform/opengl/GL2/GL2TextureManager.h"
#include "Platform/opengl/GL2/GL2Driver.h"

namespace BitEngine {

void GLVideoDriver::clearBuffer(RenderBuffer* buffer, BufferClearBitMask mask)
{
    GLbitfield bitfield = 0;
    if (mask & BufferClearBitMask::COLOR)
        bitfield |= GL_COLOR_BUFFER_BIT;
    if (mask & BufferClearBitMask::DEPTH)
        bitfield |= GL_DEPTH_BUFFER_BIT;

    GL_CHECK(glClear(bitfield));
}

void GLVideoDriver::clearBufferColor(RenderBuffer* buffer, const ColorRGBA& color)
{
    GL_CHECK(glClearColor(color.r(), color.g(), color.b(), color.a()));
}

void GLVideoDriver::setViewPort(int x, int y, int width, int height)
{
    GL_CHECK(glViewport(x, y, width, height));
}

void GLVideoDriver::configure(const Material* material)
{
    BE_PROFILE_FUNCTION();
    // BLEND
    if (material->getState(RenderConfig::BLEND) != BlendConfig::BLEND_NONE)
    {
        GL2::enableState(GL2::getGLState(RenderConfig::BLEND), true);
        if (material->getState(RenderConfig::BLEND) == BlendConfig::BLEND_ALL)
        {
            GL_CHECK(glBlendFunc(GL2::getBlendMode(material->srcColorBlendMode), GL2::getBlendMode(material->dstColorBlendMode)));
        }
        else
        {
            GL_CHECK(glBlendFuncSeparate(GL2::getBlendMode(material->srcColorBlendMode), GL2::getBlendMode(material->dstColorBlendMode),
                GL2::getBlendMode(material->srcAlphaBlendMode), GL2::getBlendMode(material->dstAlphaBlendMode)));
        }
        GL_CHECK(glBlendEquation(GL2::getBlendEquation(material->blendEquation)));
    }
    else
    {
        GL2::enableState(GL2::getGLState(RenderConfig::BLEND), false);
    }

    // FACE CULL
    if (material->getState(RenderConfig::CULL_FACE) != CULL_FACE_NONE)
    {
        GL2::enableState(GL2::getGLState(RenderConfig::CULL_FACE), true);
        switch (material->getState(RenderConfig::CULL_FACE))
        {
        case CullFaceConfig::BACK_FACE:
            GL_CHECK(glCullFace(GL_BACK));
            break;
        case CullFaceConfig::FRONT_FACE:
            GL_CHECK(glCullFace(GL_FRONT));
            break;
        case CullFaceConfig::FRONT_AND_BACK:
            GL_CHECK(glCullFace(GL_FRONT_AND_BACK));
            break;
        }
    }
    else
    {
        GL2::enableState(GL2::getGLState(RenderConfig::CULL_FACE), false);
    }

    u8 depthMode = GL2::getGLState(RenderConfig::DEPTH_TEST);
    if (depthMode &  DepthConfig::DEPTH_TEST_DISABLED) {
        GL2::enableState(GL_DEPTH_TEST, false);
    }
    else if (depthMode &  DepthConfig::DEPTH_TEST_ENABLED) {
        GL2::enableState(GL_DEPTH_TEST, true);
    }
    if (depthMode &  DepthConfig::DEPTH_TEST_WRITE_ENABLED) {
        GL_CHECK(glDepthMask(true));
    }
    else {
        GL_CHECK(glDepthMask(false));
    }
    GL2::enableState(GL2::getGLState(RenderConfig::DEPTH_TEST), material->getState(RenderConfig::DEPTH_TEST) == 0);

    GL2::enableState(GL2::getGLState(RenderConfig::MULTISAMPLE), material->getState(RenderConfig::MULTISAMPLE) == 0);
    GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_1D), material->getState(RenderConfig::TEXTURE_1D) == 0);
    GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_2D), material->getState(RenderConfig::TEXTURE_2D) == 0);
    GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_3D), material->getState(RenderConfig::TEXTURE_3D) == 0);
    GL2::enableState(GL2::getGLState(RenderConfig::TEXTURE_CUBE), material->getState(RenderConfig::TEXTURE_CUBE) == 0);
}


}