
if (BACKEND_WITH_GLFW)
    set(SRCS
        glfw/GLFW_InputSystem.cpp
        glfw/GLFW_VideoSystem.cpp
    )
    add_library(backend-glfw ${SRCS})
	set(LINK_BACKENDS "${LINK_BACKENDS} backend-glfw" PARENT_SCOPE)
endif()

if (BACKEND_WITH_GL2)
    set(SRCS
        opengl/GL2/GL2Batch.cpp
        opengl/GL2/GL2Driver.cpp
        opengl/GL2/GL2Shader.cpp
        opengl/GL2/GL2ShaderManager.cpp
        opengl/GL2/GL2TextureManager.cpp
    )
    add_library(backend-gl2 ${SRCS})
	set(LINK_BACKENDS "${LINK_BACKENDS} backend-gl2" PARENT_SCOPE)
endif()

string(STRIP "${LINK_BACKENDS}" LINK_BACKENDS)
set(LINK_BACKENDS "${LINK_BACKENDS}" PARENT_SCOPE)