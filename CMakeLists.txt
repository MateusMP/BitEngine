cmake_minimum_required(VERSION 2.8)
project (BitEngine)

# Options
set(VideoBackEnd "GLFW_WITH_GL" CACHE STRING
    "Select the video backend")
set(VideoBackEndValues GLFW_WITH_GL;GLFW_WITH_VULKAN;DX11” CACHE STRING
    "List of possible values for the VideoBackEnd cache variable")
set_property(CACHE VideoBackEnd PROPERTY STRINGS ${VideoBackEndValues})
message(STATUS "VideoBackEnd='${VideoBackEnd}'")

option(BUILD_TESTS "Enable the test build" OFF)
option(BACKEND_WITH_GLFW "Compile with GLFW backend" ON)
option(BACKEND_WITH_GL2 "Compile GL2 backend" ON)

# Directory configuration
if (BACKEND_WITH_GL2)
    find_package(GLEW REQUIRED)
    if (GLEW_FOUND)
        include_directories($(GLEW_INCLUDE_DIRS))
        link_libraries(${GLEW_LIBRARIES})
    endif()
endif()

if (BACKEND_WITH_GLFW)
    find_package(GLFW3 REQUIRED)
endif()

find_package(GLM REQUIRED)
find_package(STB REQUIRED)

# Project Configuration
set(CMAKE_BUILD_TYPE Release)
add_definitions(-std=c++1y)

# DEPENDENCIES


# BitEngine source

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/bitengine)

add_subdirectory(src)

