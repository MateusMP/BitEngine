cmake_minimum_required(VERSION 3.1.0)
project(BitEngine CXX)

# Options
set(BE_BASE_DEPENDENCY_DIRECTORY "${CMAKE_BINARY_DIR}/externals/" CACHE PATH
    "Directory where to find dependencies. We will automatically download the repositories if needed.")
set_property(CACHE BE_BASE_DEPENDENCY_DIRECTORY PROPERTY TYPE PATH)

set(BIT_ENGINE_INCLUDE_DESTINATION include)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Set the default build configuration for this build" FORCE)
endif()

message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dependency dir: ${BE_BASE_DEPENDENCY_DIRECTORY}")

option(BUILD_TESTS "Enable the test build" ON)
option(BACKEND_WITH_GLFW "Compile with GLFW backend" ON)
option(BACKEND_WITH_GL2 "Compile GL2 backend" ON)

# option(UPDATE_LIBS_ON_BUILD "Update dependencies during build step" OFF)


#### DECIDE WHAT IS NEEDED
set(NEED_NLOHMANN_JSON 1)
set(NEED_GLM 1)
set(NEED_STB 1)

if (BACKEND_WITH_GL2)
    set(NEED_GLEW 1)
endif()

if (BACKEND_WITH_GLFW)
	set(NEED_GLEW 1)
	set(NEED_GLFW 1)
endif()


## GET DEPENDENCIES
include(ExternalProject)

include("cmake/nlohmann_json.cmake")

include("cmake/glew.cmake")

if (NEED_GLFW)
    ExternalProject_Add(glfw
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		UPDATE_COMMAND ""
		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX=${CMAKE_INSTALL_PREFIX}
			-DGLFW_BUILD_EXAMPLES=OFF
			-DGLFW_BUILD_TESTS=OFF
		# LOG_DOWNLOAD 1 LOG_UPDATE 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
    )
	set(GLFW_INCLUDE ${CMAKE_INSTALL_PREFIX}/include)
	set(GLFW_LIBRARIES ${CMAKE_INSTALL_PREFIX}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
	message(STATUS "GLFW_INCLUDE='${GLFW_INCLUDE}'")
	message(STATUS "GLEW_LIBRARIES='${GLEW_LIBRARIES}'")
endif()

include("cmake/glm.cmake")

if (NEED_STB)
	ExternalProject_Add(stb
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/stb
		GIT_REPOSITORY https://github.com/nothings/stb.git
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		LOG_DOWNLOAD ON
	)
	
	# Recover project paths for additional settings.
	ExternalProject_Get_Property(stb SOURCE_DIR)
	set(STB_INCLUDE ${SOURCE_DIR})
	message(STATUS "STB_INCLUDE='${STB_INCLUDE}'")
endif()


# BitEngine source

set(BE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/)

set(ALL_BE_INCLUDES #${NLOHJSON_INCLUDE}
					#${GLEW_INCLUDE} 
					#${GLFW_INCLUDE}
					${STB_INCLUDE}
					${BE_INCLUDE_DIR}
					)

add_subdirectory(src)

# Installing

install(DIRECTORY "include/"
		DESTINATION ${BIT_ENGINE_INCLUDE_DESTINATION} FILES_MATCHING PATTERN "*.h")
install(EXPORT bitengineTargets DESTINATION lib/cmake/bitengine)

# GTest


ExternalProject_Add(googletest
	PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/gtest
	GIT_REPOSITORY    https://github.com/google/googletest.git
	GIT_TAG           aa148eb2b7f70ede0eb10de34b6254826bfb34f4
	CMAKE_ARGS		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
					-Dgtest_force_shared_crt:INTERNAL=ON
					-DBUILD_SHARED_LIBS=OFF
	TEST_COMMAND      ""
	UPDATE_COMMAND ""
	# gtest_force_shared_crt ON CACHE BOOL "" FORCE
)

ExternalProject_Get_Property(googletest INSTALL_DIR)
set(GTEST_INCLUDE ${INSTALL_DIR}/include)
set(GTEST_LIBRARIES 
			${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gtest${CMAKE_STATIC_LIBRARY_SUFFIX}
			${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}gmock_main${CMAKE_STATIC_LIBRARY_SUFFIX})
message(STATUS "GTEST_INCLUDE='${GTEST_INCLUDE}'")
message(STATUS "GTEST_LIBRARIES='${GTEST_LIBRARIES}'")

if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(Tests)
endif()
