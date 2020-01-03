#pragma once

#include "BitEngine/Core/Graphics/VideoRenderer.h"
#include "BitEngine/Core/Graphics/Texture.h"
#include "BitEngine/Core/Graphics/Shader.h"
#include "BitEngine/Core/Graphics/Material.h"
#include "BitEngine/Common/TypeDefinition.h"

#include <GL/glew.h>

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
