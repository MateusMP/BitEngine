

if (NEED_GLM)
	list(APPEND DEPENDENCIES glm_dep)

	find_package(glm PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glm NO_DEFAULT_PATH QUIET)	
	message(STATUS "GLM_INCLUDE_DIRS='${GLM_INCLUDE_DIRS}'")
	
	if (NOT ${GLM_INCLUDE_DIRS} STREQUAL "") # glm does not set GLM_FOUND
		message(STATUS "GLM_FOUND='${GLM_INCLUDE_DIRS}', ${GLM_LIBRARIES}")
		add_custom_target(glm_dep)
	else()
		ExternalProject_Add(glm_dep
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		UPDATE_COMMAND ""
		BUILD_COMMAND $(MAKE) COMMAND $(MAKE) install
		LOG_DOWNLOAD ON
		CMAKE_ARGS
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-DCMAKE_INSTALL_INCLUDEDIR=${CMAKE_INSTALL_PREFIX}/include
			-DGLM_TEST_ENABLE=OFF
		)
		message(STATUS "GLM NEEDS TO BE BUILT")
	endif()
endif()
