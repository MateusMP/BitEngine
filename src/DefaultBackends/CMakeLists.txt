
if (BITENGINE_BACKEND_WITH_GLFW)
	find_package(OpenGL REQUIRED)
	
    set(SRCS
        glfw/GLFW_InputSystem.cpp
        glfw/GLFW_VideoSystem.cpp
    )
    add_library(backend-glfw STATIC ${SRCS})
    
    include_directories(${CMAKE_SOURCE_DIR}/dependencies/glew-2.1.0/include)
	target_link_libraries(backend-glfw PUBLIC bitengine glew_s glfw ${OPENGL_LIBRARIES})
	target_include_directories(backend-glfw PUBLIC $<BUILD_INTERFACE:${BITENGINE_INCLUDE_DIR}>
												  $<INSTALL_INTERFACE:include/>)
	
	install(TARGETS backend-glfw DESTINATION lib EXPORT bitengineTargets)
	
endif()

if (BITENGINE_BACKEND_WITH_GL2)
	find_package(OpenGL REQUIRED)

    set(SRCS
		opengl/error_texture.cpp
		opengl/OpenGL.cpp
		opengl/ShaderProgram.cpp
        opengl/GL2/GL2Batch.cpp
        opengl/GL2/GL2Driver.cpp
        opengl/GL2/GL2Shader.cpp
        opengl/GL2/GL2ShaderManager.cpp
        opengl/GL2/GL2TextureManager.cpp
    )
    add_library(backend-gl2 STATIC ${SRCS})
	#target_include_directories(backend-gl2 PUBLIC dependencies/glew/glew-2.1.0/include)
	include_directories(${CMAKE_SOURCE_DIR}/dependencies/stb)
	target_include_directories(backend-gl2 PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/dependencies/glew-2.1.0/include>
												  $<INSTALL_INTERFACE:include/>)
	
	target_link_libraries(backend-gl2 PUBLIC bitengine glew_s ${OPENGL_LIBRARIES})
	target_include_directories(backend-gl2 PUBLIC $<BUILD_INTERFACE:${BITENGINE_INCLUDE_DIR}>
												  $<INSTALL_INTERFACE:include/>)
	
	install(TARGETS backend-gl2 DESTINATION lib EXPORT bitengineTargets)
endif()

string(STRIP "${BE_LINK_BACKENDS}" BE_LINK_BACKENDS)
set(BE_LINK_BACKENDS "${BE_LINK_BACKENDS}" PARENT_SCOPE)
