#pragma once

#define _BITENGINE_USE_GL4_
// #define _BITENGINE_USE_DX_ // not supported

// Force dependent libraries when needed
#ifdef _BITENGINE_USE_GL4_
	#define _BITENGINE_USE_GLEW_
	#define _BITENGINE_USE_GLFW_
#endif

#ifdef _BITENGINE_USE_GLEW_
	#include "bitengine/DefaultBackends/opengl/OpenGL.h"
#endif

#ifdef _BITENGINE_USE_GL4_
	#include "bitengine/DefaultBackends/opengl/OpenGL.h"
#endif
