if (NEED_GLFW)

	find_package(glfw3 CONFIG PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glfw3 NO_DEFAULT_PATH QUIET)
	list(APPEND DEPENDENCIES glfw3)
	
endif()
