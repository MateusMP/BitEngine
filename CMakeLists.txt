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


project(BitEngine CXX)

include("cmake/nlohmann_json.cmake")
include("cmake/glm.cmake")
include("cmake/stb.cmake")
include("cmake/glew.cmake")
include("cmake/glfw.cmake")
include("cmake/gtest.cmake")

# THEN BUILD

if (BUILD_DEPENDENCIES)
	include("cmake/bitengine.cmake")
	return()
endif()


set(BIT_ENGINE_INCLUDE_DESTINATION include)

message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dependency dir: ${BE_BASE_DEPENDENCY_DIRECTORY}")

# BitEngine source

set(BE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/)

set(ALL_BE_INCLUDES ${STB_INCLUDE}
					${BE_INCLUDE_DIR}
					)

add_subdirectory(src)

# Installing

install(DIRECTORY include/
		DESTINATION ${BIT_ENGINE_INCLUDE_DESTINATION} FILES_MATCHING PATTERN "*.h")
install(EXPORT bitengineTargets DESTINATION lib/cmake/bitengine)

# Test
if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(Tests)
endif()
