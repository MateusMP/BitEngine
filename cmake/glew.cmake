if (NEED_GLEW)

	list(APPEND DEPENDENCIES glew_dep)

	set(GLEW_USE_STATIC_LIBS ON)
	message(STATUS "GLEW_USE_STATIC_LIBS= ${GLEW_USE_STATIC_LIBS}")
	find_package(glew CONFIG PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glew NO_DEFAULT_PATH)
	
	if (TARGET GLEW::glew_s)
		message(STATUS "GLEW_FOUND!")
		add_custom_target(glew_dep)
	else()	
		ExternalProject_Add(glew_dep
			PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glew
			URL https://github.com/nigels-com/glew/releases/download/glew-2.0.0/glew-2.0.0.zip
			UPDATE_COMMAND ""
			SOURCE_SUBDIR build/cmake
			BUILD_COMMAND $(MAKE) COMMAND $(MAKE) install
			CMAKE_ARGS 
				-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
				-DBUILD_UTILS=OFF
		)
		message(STATUS "GLEW needs to be built")
	endif()
endif()
