#pragma once

#define GLEW_STATIC
#include <GL/glew.h>

#include <GLFW/glfw3.h>

enum RendererVersion{
	NOT_DEFINED = -1,
	USE_GL4 = 0,
	USE_GL3,
	USE_GL2,

	NOT_AVAILABLE,
};

RendererVersion DetectBestRenderer();