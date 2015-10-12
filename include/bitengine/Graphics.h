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

static RendererVersion DetectBestRenderer()
{
	static RendererVersion useRenderer = NOT_DEFINED;

	if (useRenderer != NOT_DEFINED){
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