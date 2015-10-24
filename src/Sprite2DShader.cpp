#include "Sprite2DShader.h"

#include "Graphics\Shaders\Sprite2DGL2.h"
#include "Graphics\Shaders\Sprite2DGL4.h"

// Shader for GL2
// Does not uses instancing!
static const char* Sprite2D_vertex_GL2 = GLSL_(120,
	attribute vec2 a_position;
	attribute vec2 a_uvcoord;

	varying vec2 fragTextureCoord;

	uniform mat4 u_viewMatrix;

	void main()
	{
		gl_Position.xy = (u_viewMatrix * vec4(a_position, 0, 1.0f)).xy;
		gl_Position.z = 0.0;
		gl_Position.w = 1.0;

		fragTextureCoord = a_uvcoord;
	}
);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* Sprite2D_fragment_GLall = GLSL_(150,
	in vec2 fragTextureCoord;

	out vec4 finalColor;

	uniform sampler2D u_texDiffuse;

	void main()
	{
		finalColor = texture(u_texDiffuse, fragTextureCoord);
	}
);
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace BitEngine{

	RendererVersion Sprite2DShader::useRenderer = NOT_DEFINED;

	Sprite2DShader::Sprite2DShader()
	{
	}

	Sprite2DShader* Sprite2DShader::CreateShader(RendererVersion forceVersion)
	{
		if (forceVersion != NOT_DEFINED)
			useRenderer = forceVersion;

		// Try to guess the best renderer
		if (useRenderer == NOT_DEFINED){
			useRenderer = DetectBestRenderer();
		}

		if (useRenderer == NOT_AVAILABLE){
			return nullptr;
		}

		// Try to compile the shaders

		// GL3 and GL4 [instancing]
		if ( (useRenderer == USE_GL4 || useRenderer == USE_GL3) )
		{
			Sprite2DGL4* s = new Sprite2DGL4(useRenderer);
			if (s->Init() == NO_ERROR)
			{
				LOGTO(Info) << "Using Sprite2D " << ((useRenderer==USE_GL4)?"GL4":"GL3") << endlog;
				return s;
			} else {
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
			if (s->Init() == NO_ERROR){
				LOGTO(Info) << "Using Sprite2D GL2" << endlog;
				return s;
			} else {
				delete s;
			}

			LOGTO(Error) << "Could not compile GL2 shader for Sprite2D, driver update needed? ERROR!" << endlog;
		}

		LOGTO(Error) << "Could not initialize Sprite2D shader!" << endlog;
		return nullptr;
	}


}