if (NEED_GLEW)

	list(APPEND DEPENDENCIES glew_dep)

	set(GLEW_USE_STATIC_LIBS ON)
	message(STATUS "GLEW_USE_STATIC_LIBS= ${GLEW_USE_STATIC_LIBS}")
    find_package(glew CONFIG PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glew NO_DEFAULT_PATH QUIET)
	
	if (TARGET GLEW::glew_s)
		message(STATUS "GLEW_FOUND!")
		add_custom_target(glew_dep)
	else()	
		
		# Fix cmake build when not on MSVC
		if (NOT "${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
			set(FORCE_MSVC_VER_FIX 0)
		else()
			set(FORCE_MSVC_VER_FIX ${MSVC_VERSION})
		endif()
	
		ExternalProject_Add(glew_dep
			PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glew
			URL https://github.com/nigels-com/glew/releases/download/glew-2.0.0/glew-2.0.0.zip
			UPDATE_COMMAND ""
			SOURCE_SUBDIR build/cmake
			BUILD_COMMAND $(MAKE) COMMAND $(MAKE) install
			CMAKE_ARGS 
				-DMSVC_VERSION=${FORCE_MSVC_VER_FIX}
				-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
				-DBUILD_UTILS=OFF
				-DBUILD_SHARED_LIBS=OFF
		)
		message(STATUS "GLEW needs to be built")
	endif()
endif()
