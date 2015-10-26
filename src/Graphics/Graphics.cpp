#include "Graphics.h"

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