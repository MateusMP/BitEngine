#pragma once

#define _BITENGINE_USE_GL4_
// #define _BITENGINE_USE_DX_ // not supported

// Force dependent libraries when needed
#ifdef _BITENGINE_USE_GL4_
	#define _USE_GLEW_
	#define _USE_GLFW_
#endif

#ifdef _USE_GLEW_
	#define GLEW_STATIC
	#include <GL/glew.h>
#endif

#ifdef _BITENGINE_USE_GL4_
	#include "DefaultBackends/opengl/OpenGL.h"
#endif
