cmake_minimum_required(VERSION 3.1.0)
project(BitEngine CXX)

# Options
set(BE_BASE_DEPENDENCY_DIRECTORY "${CMAKE_BINARY_DIR}/externals/" CACHE PATH
    "Directory where dependencies are")
set_property(CACHE BE_BASE_DEPENDENCY_DIRECTORY PROPERTY TYPE PATH)

set(LIB_INSTALL_DIR lib)
set(BIT_ENGINE_INCLUDE_DESTINATION include)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING "Set the default build configuration for this build" FORCE)
endif()

message(STATUS "VideoBackEnd='${VideoBackEnd}'")
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

if (NEED_NLOHMANN_JSON)
	ExternalProject_Add(nlohmann_json
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/nlohmann_json
		GIT_REPOSITORY https://github.com/nlohmann/json.git
		UPDATE_COMMAND ""
		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			-DBuildTests=OFF
	)
	ExternalProject_Get_Property(nlohmann_json INSTALL_DIR)
	set(NLOHJSON_INCLUDE ${INSTALL_DIR}/include/nlohmann)
	message(STATUS "NLOHJSON_INCLUDE='${NLOHJSON_INCLUDE}'")
endif()

if (NEED_GLEW)
	ExternalProject_Add(glew
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glew
		GIT_REPOSITORY https://github.com/Perlmint/glew-cmake.git
		UPDATE_COMMAND ""
		CMAKE_ARGS 
			-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
			-Dglew-cmake_BUILD_SHARED=OFF
	)
	ExternalProject_Get_Property(glew INSTALL_DIR)
	set(GLEW_INCLUDE ${INSTALL_DIR}/include)
	set(GLEW_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glew${CMAKE_STATIC_LIBRARY_SUFFIX})
	message(STATUS "GLEW_INCLUDE='${GLEW_INCLUDE}'")
	message(STATUS "GLEW_LIBRARIES='${GLEW_LIBRARIES}'")
endif()

if (NEED_GLFW)
    ExternalProject_Add(glfw
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glfw
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
	set(GLFW_LIBRARIES ${INSTALL_DIR}/lib/${CMAKE_STATIC_LIBRARY_PREFIX}glfw3${CMAKE_STATIC_LIBRARY_SUFFIX})
	message(STATUS "GLFW_INCLUDE='${GLFW_INCLUDE}'")
	message(STATUS "GLEW_LIBRARIES='${GLEW_LIBRARIES}'")
endif()

if (NEED_GLM)
	ExternalProject_Add(glm
		PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/glm
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
	message(STATUS "GLM_INCLUDE='${GLM_INCLUDE}'")
endif()


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




# Project Configuration
add_definitions(-static-libstdc++)

# BitEngine source

set(ALL_BE_INCLUDES ${NLOHJSON_INCLUDE}
					${GLEW_INCLUDE} 
					${GLFW_INCLUDE}
					${GLM_INCLUDE} 
					${STB_INCLUDE}
					${CMAKE_CURRENT_SOURCE_DIR}/include/)

add_subdirectory(src)


# Installing

install(DIRECTORY "include/"
DESTINATION ${BIT_ENGINE_INCLUDE_DESTINATION} FILES_MATCHING PATTERN "*.h")


# GTest


ExternalProject_Add(googletest
	PREFIX ${BE_BASE_DEPENDENCY_DIRECTORY}/gtest
	GIT_REPOSITORY    https://github.com/google/googletest.git
	GIT_TAG           0e0ff5c3410f88e09701fef04dcfcaad9cd3494f
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

    add_executable(TestBuild
        src/Tests/test_build.cpp)
    add_test(BuildTests TestBuild)
    target_link_libraries(TestBuild ${GTEST_LIBRARIES} -static-libstdc++)
    target_include_directories(TestBuild PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

    add_executable(TestCore
            src/Tests/reflectiontest.cpp)
    add_test(ReflectionTests TestCore)
    target_link_libraries(TestCore bitengine ${GTEST_LIBRARIES} -static-libstdc++)
    target_include_directories(TestCore PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})

    add_executable(TestCommonUtils
            src/Tests/commonTests.cpp
            src/Tests/vectorBoolTest.cpp)
    add_test(CommonUtilsTests TestCommonUtils)
    target_link_libraries(TestCommonUtils ${GTEST_LIBRARIES} -static-libstdc++)
    target_include_directories(TestCommonUtils PUBLIC ${GTEST_INCLUDE} ${ALL_BE_INCLUDES})
endif()
