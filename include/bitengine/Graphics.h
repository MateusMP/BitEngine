#pragma once

#define _BITENGINE_USE_GL4_

// Force dependent libraries when needed
#ifdef _BITENGINE_USE_GL4_
	#define _USE_GLEW_
	#define _USE_GLFW_
#endif

#ifdef _USE_GLEW_
	#define GLEW_STATIC
	#include <GL/glew.h>
#endif

#ifdef _USE_GLFW_
	#include <GLFW/glfw3.h>
#endif

#ifdef _BITENGINE_USE_GL4_
	#include "Video/opengl/OpenGL4Renderer.h"
#endif

enum RendererVersion{
	NOT_DEFINED = -1,
	USE_GL4 = 0,
	USE_GL3,
	USE_GL2,

	NOT_AVAILABLE,
};

RendererVersion DetectBestRenderer();