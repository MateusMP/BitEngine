#pragma once

#include <glad/glad.h>
#include <BitEngine/Core/Logger.h>
#include <BitEngine/Core/Assert.h>


static void _gl_debug_log(const char* message) {
    int a__glerr = glGetError();
    if (a__glerr != GL_NO_ERROR) {
        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << message << std::hex << a__glerr;
    }
}

#ifdef BE_DEBUG
#define GL_CHECK(call)        \
    _gl_debug_log("GL ERROR (had unhandled error): "); \
	call;		                            \
	_gl_debug_log("GL ERROR: ")
#else
    #define GL_CHECK(call)        call
#endif

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)