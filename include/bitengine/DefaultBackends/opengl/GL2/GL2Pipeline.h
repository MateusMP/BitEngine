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


	class OpenGL2Adapter : public GLAdapter
	{
		public:
		OpenGL2Adapter(GameEngine* ge) : engine(ge) {}
		~OpenGL2Adapter() {}

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

		bool init() override
		{
			shaderManager = new GL2ShaderManager();
			textureManager = new GL2TextureManager();
			engine->getResourceLoader()->registerResourceManager("SHADER", shaderManager);
			engine->getResourceLoader()->registerResourceManager("TEXTURE", textureManager);

			return true;
		}

		u32 getVideoAdapter() override
		{
			return OPENGL_2;
		}

		private:
			GameEngine* engine;
			GL2ShaderManager* shaderManager;
			GL2TextureManager* textureManager;
	};


}