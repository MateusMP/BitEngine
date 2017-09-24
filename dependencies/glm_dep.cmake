if (NEED_GLM)
	find_package(glm PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glm NO_DEFAULT_PATH QUIET)	
	message(STATUS "GLM_INCLUDE_DIRS='${GLM_INCLUDE_DIRS}'")	
endif()
