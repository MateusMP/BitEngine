cmake_minimum_required(VERSION 3.1.0)

include (ExternalProject)
project(BitEngine CXX)

option(BITENGINE_BUILD_TESTS "Enable the test build" ON)
option(BITENGINE_BUILD_SAMPLES "Build samples?" ON)
option(BITENGINE_BACKEND_WITH_GLFW "Compile with GLFW backend" ON)
option(BITENGINE_BACKEND_WITH_GL2 "Compile GL2 backend" ON)

#### DECIDE WHAT IS NEEDED
set(NEED_NLOHMANN_JSON 1)
set(NEED_GLM 1)
set(NEED_STB 1)

if (BITENGINE_BACKEND_WITH_GL2)
    set(NEED_GLEW 1)
endif()

if (BITENGINE_BUILD_SAMPLES)
    set(NEED_ASSIMP 1)
endif()

if (BITENGINE_BACKEND_WITH_GLFW)
	set(NEED_GLEW 1)
	set(NEED_GLFW 1)
endif()

add_subdirectory(dependencies/)

set(BITENGINE_INCLUDE_DESTINATION include)

message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Dependency dir: ${BE_BASE_DEPENDENCY_DIRECTORY}")

# BitEngine source

set(BITENGINE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include/)

add_subdirectory(src)

# Installing

install(DIRECTORY include/
		DESTINATION ${BITENGINE_INCLUDE_DESTINATION} FILES_MATCHING PATTERN "*.h")
install(EXPORT bitengineTargets DESTINATION lib/cmake/bitengine)

# Build Samples (Or your game!)
# Samples are built as DLLs
add_subdirectory(samples/sample01)

# Test
if (BITENGINE_BUILD_TESTS)
	enable_testing()
	add_subdirectory(Tests)
endif()
