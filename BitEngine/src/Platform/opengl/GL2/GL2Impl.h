#pragma once

#include "Platform/opengl/GL2/OpenGL2.h"
#include "BitEngine/Core/Graphics/VideoRenderer.h"

namespace BitEngine {
class GL2
{
public:

    static void bindVbo(GLuint vbo) {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, vbo));
    }
    static void unbindVbo() {
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    }

    static void loadBufferRange(const void* data, u32 offset, u32 len, GLenum mode) {
        const char* d = static_cast<const char*>(data);
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, nullptr, mode));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, d + offset, mode));
    }

    static void bindVao(GLuint vao) {
        GL_CHECK(glBindVertexArray(vao));
    }

    static void unbindVao() {
        GL_CHECK(glBindVertexArray(0));
    }

    static void genVao(GLsizei num, GLuint* _array) {
        GL_CHECK(glGenVertexArrays(num, _array));
    }

    static void genVbo(GLsizei num, GLuint* _array) {
        GL_CHECK(glGenBuffers(num, _array));
    }

    static void setupVbo(GLuint attrIndex, GLuint vbo, GLint size, GLenum dataType, GLboolean normalized, GLsizei stride, u64 offset, GLuint divisor) {
        bindVbo(vbo);
        GL_CHECK(glVertexAttribPointer(attrIndex, size, dataType, normalized, stride, (const GLvoid*)offset));
        // GL_CHECK(glVertexAttribDivisor(attrIndex, divisor));
        GL_CHECK(glEnableVertexAttribArray(attrIndex));
    }

    static void deleteVaos(u32 num, GLuint* _array) {
        GL_CHECK(glDeleteVertexArrays(num, _array));
    }

    static void bindShaderProgram(GLuint program) {
        GL_CHECK(glUseProgram(program));
    }

    static void enableState(GLenum state, bool enable) {
        if (enable) {
            GL_CHECK(glEnable(state));
        }
        else {
            GL_CHECK(glDisable(state));
        }
    }

    static GLenum toGLType(DataType dt)
    {
        static constexpr GLenum glTypes[] = {
            0,
            GL_SAMPLER_1D,
            GL_SAMPLER_2D,
            GL_SAMPLER_3D,
            GL_SAMPLER_CUBE,

            GL_INT,
            GL_FLOAT,

            GL_FLOAT_VEC2,
            GL_FLOAT_VEC3,
            GL_FLOAT_VEC4,

            GL_FLOAT_MAT2,
            GL_FLOAT_MAT3,
            GL_FLOAT_MAT4,
        };
        BE_ASSERT(dt.value < DataType::TOTAL_DATA_TYPES);
        return glTypes[dt.value];
    }

    static GLenum fromGLTypeToGLDataType(GLenum e)
    {
        switch (e)
        {
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
            return GL_INT;

        case GL_FLOAT_MAT4:
        case GL_FLOAT_MAT3:
        case GL_FLOAT_MAT2:
        case GL_FLOAT_VEC4:
        case GL_FLOAT_VEC3:
        case GL_FLOAT_VEC2:
        case GL_FLOAT:
            return GL_FLOAT;
        default:
            BE_ASSERT(e && 0 && "Unexpected type");
            return e;
        }
    }

    static GLenum fromGLTypeToGLDataTypeSize(GLenum e)
    {
        switch (e)
        {
        case GL_FLOAT:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
            return 1;
        case GL_FLOAT_MAT3:
        case GL_FLOAT_VEC3:
            return 3;
        case GL_FLOAT_MAT4:
        case GL_FLOAT_VEC4:
            return 4;
        case GL_FLOAT_MAT2:
        case GL_FLOAT_VEC2:
            return 2;
        default:
            BE_ASSERT(e && 0 && "Unexpected type");
            return e;
        }
    }

    static u32 sizeOfGlType(GLenum type)
    {
        switch (type)
        {
        case GL_FLOAT:
            return sizeof(GLfloat);

        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            return sizeof(GLbyte);

        case GL_UNSIGNED_SHORT:
        case GL_SHORT:
            return sizeof(GLshort);

        case GL_UNSIGNED_INT:
        case GL_INT:
            return sizeof(GLint);

        case GL_HALF_FLOAT:
            return sizeof(GLhalf);

        case GL_DOUBLE:
            return sizeof(GLdouble);
        }

        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "sizeOfType not defined for " << type;

        return 4;
    }

    static GLenum getBlendMode(BlendFunc mode)
    {
        static constexpr GLenum blendModes[BlendFunc::TOTAL_BLEND_MODES] = {
            GL_ZERO,
            GL_ONE,
            GL_SRC_COLOR,
            GL_ONE_MINUS_SRC_COLOR,
            GL_DST_COLOR,
            GL_ONE_MINUS_DST_COLOR,
            GL_SRC_ALPHA,
            GL_ONE_MINUS_SRC_ALPHA,
            GL_DST_ALPHA,
            GL_ONE_MINUS_DST_ALPHA,
            GL_CONSTANT_COLOR,
            GL_ONE_MINUS_CONSTANT_COLOR,
            GL_CONSTANT_ALPHA,
            GL_ONE_MINUS_CONSTANT_ALPHA,
            GL_SRC_ALPHA_SATURATE
        };
        BE_ASSERT(mode < BlendFunc::TOTAL_BLEND_MODES);
        return blendModes[mode];
    }

    static GLenum getBlendEquation(BlendEquation mode)
    {
        static constexpr GLenum blendEquation[BlendEquation::TOTAL_BLEND_EQUATIONS] = {
            GL_FUNC_ADD,
            GL_FUNC_SUBTRACT,
            GL_FUNC_REVERSE_SUBTRACT,
        };
        BE_ASSERT(mode < BlendEquation::TOTAL_BLEND_EQUATIONS);
        return blendEquation[mode];
    }

    static GLenum getGLState(RenderConfig config)
    {
        static constexpr GLenum configs[(u8)RenderConfig::TOTAL_RENDER_CONFIGS] = {
            GL_BLEND,

            GL_CULL_FACE,
            GL_DEPTH_TEST,

            GL_MULTISAMPLE,

            GL_TEXTURE_1D,
            GL_TEXTURE_2D,
            GL_TEXTURE_3D,
            GL_TEXTURE_CUBE_MAP,
        };
        BE_ASSERT(config < RenderConfig::TOTAL_RENDER_CONFIGS);
        return configs[(u8)config];
    }
};
}
