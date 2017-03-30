
if (BACKEND_WITH_GLFW)
	find_package(OpenGL REQUIRED)
	
    set(SRCS
        glfw/GLFW_InputSystem.cpp
        glfw/GLFW_VideoSystem.cpp
    )
    add_library(backend-glfw STATIC ${SRCS})
	target_compile_definitions(backend-glfw PUBLIC -DGLEW_STATIC)
	message(STATUS "backend-glfw link with='${OPENGL_LIBRARIES} ${BE_INCLUDE_DIR}'")
	target_link_libraries(backend-glfw glfw3 libglew_static ${OPENGL_LIBRARIES})
	target_include_directories(backend-glfw PUBLIC $<BUILD_INTERFACE:${BE_INCLUDE_DIR}>
												  $<INSTALL_INTERFACE:include/>)
	target_include_directories(backend-glfw PRIVATE ${ALL_BE_INCLUDES})
	set(BE_LINK_BACKENDS "${BE_LINK_BACKENDS} backend-glfw" PARENT_SCOPE)
	
	install(TARGETS backend-glfw DESTINATION lib EXPORT bitengineTargets)
	
endif()

if (BACKEND_WITH_GL2)
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
	target_compile_definitions(backend-gl2 PUBLIC -DGLEW_STATIC)
	message(STATUS "backend-gl2 link with='${OPENGL_LIBRARIES}'")
	target_link_libraries(backend-gl2 libglew_static ${OPENGL_LIBRARIES})
	target_include_directories(backend-gl2 PUBLIC $<BUILD_INTERFACE:${BE_INCLUDE_DIR}>
												  $<INSTALL_INTERFACE:include/>)
	target_include_directories(backend-gl2 PRIVATE ${ALL_BE_INCLUDES})
	
	set(BE_LINK_BACKENDS "${BE_LINK_BACKENDS} backend-gl2" PARENT_SCOPE)
	
	install(TARGETS backend-gl2 DESTINATION lib EXPORT bitengineTargets)
endif()

string(STRIP "${BE_LINK_BACKENDS}" BE_LINK_BACKENDS)
set(BE_LINK_BACKENDS "${BE_LINK_BACKENDS}" PARENT_SCOPE)
