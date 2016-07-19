#pragma once

#ifdef _BITENGINE_USE_GLEW_STATIC_
    #define GLEW_STATIC
#endif
#include <GL/glew.h>

#define GL_CHECK(call)				\
	if (BE_DEBUG){								\
	int a__glerr = glGetError();	\
	if (a__glerr != GL_NO_ERROR) { LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR (had unhandled error): " << std::hex << a__glerr; } }	\
	call;	if (BE_DEBUG) {				\
	int a__glerr = glGetError();		\
	if (a__glerr != GL_NO_ERROR) { LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR: " << std::hex << a__glerr; } }

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)