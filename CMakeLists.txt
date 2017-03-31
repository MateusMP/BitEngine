cmake_minimum_required(VERSION 3.1.0)

include (ExternalProject)

option(BUILD_DEPENDENCIES "Build dependencies first?" ON)
option(BUILD_TESTS "Enable the test build" ON)
option(BACKEND_WITH_GLFW "Compile with GLFW backend" ON)
option(BACKEND_WITH_GL2 "Compile GL2 backend" ON)

# Options
set(BE_BASE_DEPENDENCY_DIRECTORY "${CMAKE_BINARY_DIR}/externals/" CACHE PATH
    "Directory where to find dependencies. We will automatically download the repositories if needed.")
set_property(CACHE BE_BASE_DEPENDENCY_DIRECTORY PROPERTY TYPE PATH)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Set the default build configuration for this build" FORCE)
endif()

set (DEPENDENCIES)

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

include("cmake/nlohmann_json.cmake")
include("cmake/glm.cmake")
include("cmake/stb.cmake")
include("cmake/glew.cmake")
include("cmake/glfw.cmake")

# THEN BUILD

if (BUILD_DEPENDENCIES)
	project(BitEngine NONE)
	include("cmake/bitengine.cmake")
	return()
endif()

project(BitEngine CXX)

set(BIT_ENGINE_INCLUDE_DESTINATION include)

message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dependency dir: ${BE_BASE_DEPENDENCY_DIRECTORY}")

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
