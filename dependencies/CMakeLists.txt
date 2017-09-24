

add_subdirectory(glm)

#add_subdirectory(gl3w)
add_subdirectory(glew-2.1.0/build/cmake)

set(BuildTests OFF CACHE BOOL "enable X functionality")
add_subdirectory(json)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
add_subdirectory(glfw)

option( ASSIMP_BUILD_SAMPLES "If the official samples are built as well (needs Glut)." OFF)
option( ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF)
add_subdirectory(assimp)

option(GTEST_LINKED_AS_SHARED_LIBRARY "" ON)
add_subdirectory(googletest)