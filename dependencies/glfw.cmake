if (NEED_GLFW)

	list(APPEND DEPENDENCIES glfw_dep)
	
	find_package(glfw3 CONFIG PATHS ${CMAKE_INSTALL_PREFIX}/lib/cmake/glfw3 NO_DEFAULT_PATH QUIET)
	
	if (TARGET glfw)
        message(STATUS "GLFW_FOUND!")
		add_custom_target(glfw_dep)
	else()
        message(STATUS "GLFW NOT FOUND!")
        ExternalProject_Add(glfw_dep
            PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            UPDATE_COMMAND ""
            CMAKE_ARGS 
                -DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
                -DGLFW_BUILD_EXAMPLES=OFF
                -DGLFW_BUILD_TESTS=OFF
            # LOG_DOWNLOAD 1 LOG_UPDATE 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
        )
        #set(GLFW_INCLUDE ${CMAKE_INSTALL_PREFIX}/include)
        #set(GLFW_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
        #message(STATUS "GLFW_INCLUDE='${GLFW_INCLUDE}'")
        #message(STATUS "GLFW_LIBRARIES='${GLFW_LIBRARIES}'")	
	endif()
	
endif()
