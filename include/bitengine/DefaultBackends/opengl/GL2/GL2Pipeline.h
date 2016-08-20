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