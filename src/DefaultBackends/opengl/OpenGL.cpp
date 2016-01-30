#include <string>

#include "DefaultBackends/opengl/OpenGL.h"

void _check_gl_error(const char *file, int line)
{
	GLenum err(glGetError());

	while (err != GL_NO_ERROR) {
		std::string error;

		switch (err) {
		case GL_INVALID_OPERATION:      error = "INVALID_OPERATION";      break;
		case GL_INVALID_ENUM:           error = "INVALID_ENUM";           break;
		case GL_INVALID_VALUE:          error = "INVALID_VALUE";          break;
		case GL_OUT_OF_MEMORY:          error = "OUT_OF_MEMORY";          break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:  error = "INVALID_FRAMEBUFFER_OPERATION";  break;
		default: error = "Code " + std::to_string(err) + " d";
		}

		LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "ERROR: GL_" << error.c_str() << " - " << file << ":" << line << "\a" << std::endl;
		err = glGetError();
	}
}

RendererVersion DetectBestRenderer()
{
	static RendererVersion useRenderer = NOT_DEFINED;

	if (useRenderer != NOT_DEFINED) {
		return useRenderer;
	}

	if (glewIsSupported("GL_VERSION_4_2"))
	{
		useRenderer = RendererVersion::USE_GL4;
	}
	else if (glewIsSupported("GL_VERSION_3_0"))
	{
		useRenderer = RendererVersion::USE_GL3;
	}
	else
	{
		useRenderer = RendererVersion::USE_GL2;
	}

	return useRenderer;
}