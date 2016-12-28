cmake_minimum_required(VERSION 3.0)
project (BitEngine)

# Options
set(VideoBackEnd "GLFW_WITH_GL" CACHE STRING
    "Select the video backend")
set(VideoBackEndValues GLFW_WITH_GL;GLFW_WITH_VULKAN;DX11” CACHE STRING
    "List of possible values for the VideoBackEnd cache variable")
set_property(CACHE VideoBackEnd PROPERTY STRINGS ${VideoBackEndValues})


if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Set the default build configuration for this build" FORCE)
endif()

message(STATUS "VideoBackEnd='${VideoBackEnd}'")
message(STATUS "Installing libs in: ${CMAKE_BINARY_DIR}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

option(BUILD_TESTS "Enable the test build" OFF)
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

if (NEED_NLOHMANN_JSON)
	ExternalProject_Add(nlohmann_json
		PREFIX ${CMAKE_BINARY_DIR}/nlohmann_json
		GIT_REPOSITORY https://github.com/nlohmann/json.git
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
	)
	ExternalProject_Get_Property(nlohmann_json SOURCE_DIR)
	set(NLOHJSON_INCLUDE ${SOURCE_DIR}/src)
	include_directories(${NLOHJSON_INCLUDE})
	message(STATUS "NLOHJSON_INCLUDE='${NLOHJSON_INCLUDE}'")
endif()

if (NEED_GLEW)
	ExternalProject_Add(glew
		PREFIX ${CMAKE_BINARY_DIR}/glew
		GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
		UPDATE_COMMAND ""
		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			-Dglew-cmake_BUILD_SHARED=OFF
	)
	ExternalProject_Get_Property(glew INSTALL_DIR)
	set(GLEW_INCLUDE ${INSTALL_DIR}/include)
	set(GLEW_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glew${CMAKE_STATIC_LIBRARY_SUFFIX})
	include_directories(${GLEW_INCLUDE})
	message(STATUS "GLEW_INCLUDE='${GLEW_INCLUDE}'")
	message(STATUS "GLEW_LIBRARIES='${GLEW_LIBRARIES}'")
endif()

if (NEED_GLFW)
    ExternalProject_Add(glfw
		PREFIX ${CMAKE_BINARY_DIR}/glfw
		GIT_REPOSITORY https://github.com/glfw/glfw.git
		UPDATE_COMMAND ""
		CMAKE_ARGS 
			-DGLFW_BUILD_EXAMPLES=OFF
			-DGLFW_BUILD_TESTS=OFF
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
		# LOG_DOWNLOAD 1 LOG_UPDATE 1 LOG_CONFIGURE 1 LOG_BUILD 1 LOG_INSTALL 1
    )
	ExternalProject_Get_Property(glfw INSTALL_DIR)
	set(GLFW_INCLUDE ${INSTALL_DIR}/include)
	set(GLEW_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
	include_directories(${GLFW_INCLUDE})
	message(STATUS "GLFW_INCLUDE='${GLFW_INCLUDE}'")
	message(STATUS "GLEW_LIBRARIES='${GLEW_LIBRARIES}'")
endif()

if (NEED_GLM)
	ExternalProject_Add(glm
		PREFIX ${CMAKE_BINARY_DIR}/glm
		GIT_REPOSITORY https://github.com/g-truc/glm.git
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		#${CMAKE_COMMAND} -E copy_directory <SOURCE_DIR>/glm <INSTALL_DIR>/include
		LOG_DOWNLOAD ON
		)
	ExternalProject_Get_Property(glm SOURCE_DIR)
	set(GLM_INCLUDE ${SOURCE_DIR})
	include_directories(${GLM_INCLUDE})
	message(STATUS "GLM_INCLUDE='${GLM_INCLUDE}'")
endif()


if (NEED_STB)
	ExternalProject_Add(stb_ext
		PREFIX ${CMAKE_BINARY_DIR}/stb_ext
		GIT_REPOSITORY https://github.com/nothings/stb.git
		UPDATE_COMMAND ""
		CONFIGURE_COMMAND ""
		BUILD_COMMAND ""
		INSTALL_COMMAND ""
		LOG_DOWNLOAD ON
	)
	
	# Recover project paths for additional settings.
	ExternalProject_Get_Property(stb_ext SOURCE_DIR)
	set(STB_INCLUDE ${SOURCE_DIR})
	include_directories(${STB_INCLUDE})
	message(STATUS "STB_INCLUDE='${STB_INCLUDE}'")
endif()

# Project Configuration
set(CMAKE_BUILD_TYPE Release)
add_definitions(-std=c++1y)

# BitEngine source

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/bitengine)

add_subdirectory(src)


# GTest

ExternalProject_Add(googletest
  GIT_REPOSITORY    https://github.com/google/googletest.git
  GIT_TAG           master
  CMAKE_ARGS		-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  TEST_COMMAND      ""
  UPDATE_COMMAND ""
)

ExternalProject_Get_Property(googletest INSTALL_DIR)
set(GTEST_INCLUDE ${INSTALL_DIR}/include)
include_directories(${GTEST_INCLUDE})
set(GTEST_LIBRARIES ${INSTALL_DIR}/lib/libgmock.a ${INSTALL_DIR}/lib/libgmock_main.a)
message(STATUS "GTEST_INCLUDE='${GTEST_INCLUDE}'")
message(STATUS "GTEST_LIBRARIES='${GTEST_LIBRARIES}'")

enable_testing()
add_executable(Test
    src/Tests/test_build.cpp
	src/Tests/vectorBoolTest.cpp)
add_test(GeneralTests Test)
target_link_libraries(Test ${GTEST_LIBRARIES})