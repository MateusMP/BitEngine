if (NEED_GLEW)

	set(GLEW_USE_STATIC_LIBS ON)
	include(${CMAKE_INSTALL_PREFIX}/lib/cmake/glew/glewTargets.cmake)
	if (TARGET GLEW::glew_s)
		message(STATUS "GLEW_FOUND!")
	else()	
		ExternalProject_Add(glew
			PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glew
			GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
			UPDATE_COMMAND ""
			CMAKE_ARGS 
				-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
				-Dglew-cmake_BUILD_SHARED=OFF
		)
		ExternalProject_Get_Property(glew INSTALL_DIR)
		#set(GLEW_INCLUDE ${INSTALL_DIR}/include)
		#set(GLEW_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glew${CMAKE_STATIC_LIBRARY_SUFFIX})
		message(STATUS "GLEW_INCLUDE='${GLEW_INCLUDE}'")
		#message(FATAL_ERROR "Reconfigure now that glew is built.")
	endif()
endif()
