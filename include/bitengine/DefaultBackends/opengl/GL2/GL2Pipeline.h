#pragma once

#include <algorithm>
#include <set>
#include <unordered_map>

#include "Common/MathUtils.h"
#include "Core/Logger.h"

#include "OpenGL2.h"

#include "GL2Shader.h"
#include "GL2Structs.h"
#include "GL2TextureManager.h"
#include "GL2Batch.h"
#include "GL2ShaderManager.h"

namespace BitEngine {


	class OpenGL2Renderer : public VideoRenderer
	{
		public:
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

		~OpenGL2Renderer() {}

		bool init() override
		{
			shaderManager = new GL2ShaderManager();
			textureManager = new GL2TextureManager();

			return true;
		}

		u32 getGraphicPipelineType() override
		{
			return OPENGL_2;
		}

		ResourceManager* getShaderManager() override
		{
			return shaderManager;
		}

		ResourceManager* getTextureManager() override
		{
			return textureManager;
		}

		void ClearBuffer(IRenderBuffer* buffer, BufferClearBitMask mask) override
		{
			GLbitfield bitfield = 0;
			if (mask & BufferClearBitMask::COLOR)
				bitfield |= GL_COLOR_BUFFER_BIT;
			if (mask & BufferClearBitMask::DEPTH)
				bitfield |= GL_DEPTH_BUFFER_BIT;

			glClear(bitfield);
		}

		void ClearBufferColor(IRenderBuffer* buffer, const ColorRGBA& color) override
		{
			glClearColor(color.r(), color.g(), color.b(), color.a());
		}

		void setViewPort(int x, int y, int width, int height) override
		{
			glViewport(x, y, width, height);
		}

		private:
		GL2ShaderManager* shaderManager;
		GL2TextureManager* textureManager;
	};


}