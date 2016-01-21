#include "Core/Sprite2DShader.h"

#include "DefaultBackends/opengl/ShaderProgram.h"

static const char* Sprite2D_fragment_GLall = GLSL_(150,
	in vec2 fragTextureCoord;

out vec4 finalColor;

uniform sampler2D u_texDiffuse;

void main()
{
	finalColor = texture(u_texDiffuse, fragTextureCoord);
}
);

#include "DefaultBackends/opengl/Sprite2DGL2.h"
#include "DefaultBackends/opengl/Sprite2DGL4.h"

namespace BitEngine {

	RendererVersion Sprite2DShader::useRenderer = NOT_DEFINED;

	Sprite2DShader::Sprite2DShader()
	{
	}

	Sprite2DShader* Sprite2DShader::CreateShader(RendererVersion forceVersion)
	{
		if (forceVersion != NOT_DEFINED)
			useRenderer = forceVersion;

		// Try to guess the best renderer
		if (useRenderer == NOT_DEFINED) {
			useRenderer = DetectBestRenderer();
		}

		if (useRenderer == NOT_AVAILABLE) {
			return nullptr;
		}

		// Try to compile the shaders

		// GL3 and GL4 [instancing]
		if ((useRenderer == USE_GL4 || useRenderer == USE_GL3))
		{
			Sprite2DGL4* s = new Sprite2DGL4(useRenderer);
			if (s->Init() == NO_ERROR)
			{
				LOGTO(Info) << "Using Sprite2D " << ((useRenderer == USE_GL4) ? "GL4" : "GL3") << endlog;
				return s;
			}
			else {
				delete s;
			}

			// If failed to compile -> fallback to GL2
			LOGTO(Warning) << "Could not compile GL3/GL4 shader for Sprite2D, driver update needed? Fallback to GL2..." << endlog;
			useRenderer = USE_GL2;
		}

		// GL2
		if (useRenderer == USE_GL2)
		{
			Sprite2DGL2* s = new Sprite2DGL2();
			if (s->Init() == NO_ERROR) {
				LOGTO(Info) << "Using Sprite2D GL2" << endlog;
				return s;
			}
			else {
				delete s;
			}

			LOGTO(Error) << "Could not compile GL2 shader for Sprite2D, driver update needed? ERROR!" << endlog;
		}

		LOGTO(Error) << "Could not initialize Sprite2D shader!" << endlog;
		return nullptr;
	}


}