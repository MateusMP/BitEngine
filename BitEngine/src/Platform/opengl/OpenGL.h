#pragma once

#include <glad/glad.h>
#include <BitEngine/Core/Logger.h>
#include <BitEngine/Core/Assert.h>

#define GL_CHECK(call)              \
	if (BE_DEBUG){                  \
	    int a__glerr = glGetError();	\
	    if (a__glerr != GL_NO_ERROR) { \
		    LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR (had unhandled error): " << std::hex << a__glerr; BE_ASSERT(false); } }	\
	call;		                            \
	if (BE_DEBUG) {                         \
	    int a__glerr = glGetError();        \
	    if (a__glerr != GL_NO_ERROR) {      \
		    LOG(EngineLog, BE_LOG_ERROR) << "GL ERROR: " << std::hex << a__glerr; BE_ASSERT(false); } }

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)