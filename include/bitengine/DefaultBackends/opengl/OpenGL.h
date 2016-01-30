#pragma once

#include "OpenGLheaders.h"

#define check_gl_error() _check_gl_error(__FILE__,__LINE__)

void _check_gl_error(const char *file, int line);

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)


enum RendererVersion {
	NOT_DEFINED = -1,
	USE_GL4 = 0,
	USE_GL3,
	USE_GL2,

	NOT_AVAILABLE,
};

RendererVersion DetectBestRenderer();

#include "ShaderProgram.h"
#include "VertexArrayObject.h"
#include "OpenGLRenderer.h"
#include "GLTextureManager.h"