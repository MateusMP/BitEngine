cmake_minimum_required(VERSION 3.1.0)

include (ExternalProject)
project(BitEngine CXX)

option(BUILD_TESTS "Enable the test build" ON)
option(BUILD_SAMPLES "Build samples?" ON)
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

if (BUILD_SAMPLES)
    set(NEED_ASSIMP 1)
endif()

if (BACKEND_WITH_GLFW)
	set(NEED_GLEW 1)
	set(NEED_GLFW 1)
endif()

add_subdirectory(dependencies/)

include("dependencies/nlohmann_json_dep.cmake")
include("dependencies/glm_dep.cmake")
include("dependencies/stb_dep.cmake")
include("dependencies/glew_dep.cmake")
include("dependencies/glfw_dep.cmake")
include("dependencies/gtest_dep.cmake")
include("dependencies/assimp_dep.cmake")

set(BIT_ENGINE_INCLUDE_DESTINATION include)

message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dependency dir: ${BE_BASE_DEPENDENCY_DIRECTORY}")

# BitEngine source


set(BE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/)

add_subdirectory(src)

# Installing

install(DIRECTORY include/
		DESTINATION ${BIT_ENGINE_INCLUDE_DESTINATION} FILES_MATCHING PATTERN "*.h")
install(EXPORT bitengineTargets DESTINATION lib/cmake/bitengine)

# Build Samples (Or your game!)
# Samples are built as DLLs
add_subdirectory(samples/sample01)

# Test
if (BUILD_TESTS)
	enable_testing()
	add_subdirectory(Tests)
endif()
