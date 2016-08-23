#pragma once

#include "OpenGL_Headers.h"
#include "Core/Logger.h"

enum RendererVersion {
	NOT_DEFINED = -1,
	USE_GL4 = 0,
	USE_GL3,
	USE_GL2,

	NOT_AVAILABLE,
};

RendererVersion DetectBestRenderer();

#include "GL2/GL2Pipeline.h"