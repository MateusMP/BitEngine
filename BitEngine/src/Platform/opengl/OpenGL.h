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

#define GL_CHECK(call)              \
	if (BE_DEBUG){ _gl_debug_log("GL ERROR (had unhandled error): "); } \
	call;		                            \
	if (BE_DEBUG) {_gl_debug_log("GL ERROR: ");}

#define GL_CALL_AVAILABLE(call) ((*call)!=nullptr)