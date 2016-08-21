#pragma once

#include "Common/TypeDefinition.h"

#ifdef _BITENGINE_USE_GLEW_STATIC_
    #define GLEW_STATIC
#endif
#include <GL/glew.h>

static void GL_BREAK() {
	throw "GL Failed";
}

#define GL_CHECK(call)				\
	if (BE_DEBUG){								\
	int a__glerr = glGetError();	\
	if (a__glerr != GL_NO_ERROR) { \
		LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR (had unhandled error): " << std::hex << a__glerr; GL_BREAK(); } }	\
	call;		\
	if (BE_DEBUG) {				\
	int a__glerr = glGetError();		\
	if (a__glerr != GL_NO_ERROR) {		\
		LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR: " << std::hex << a__glerr; GL_BREAK(); } }

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)


class GLAdapter
{
	public:
	virtual ~GLAdapter() {}
	virtual bool init() = 0;

	virtual u32 getVideoAdapter() = 0;
};
