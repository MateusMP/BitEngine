# Force build libraries in release mode
set( CMAKE_BUILD_TYPE_COPY "${CMAKE_BUILD_TYPE}" )
set( CMAKE_BUILD_TYPE "Release" )


add_subdirectory(glm)

option(BUILD_UTILS "utilities" OFF)
add_subdirectory(glew/build/cmake)

set(BuildTests OFF CACHE BOOL "enable X functionality")
add_subdirectory(json)

option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" OFF)
option(GLFW_BUILD_TESTS "Build the GLFW test programs" OFF)
add_subdirectory(glfw)

option( ASSIMP_BUILD_ASSIMP_TOOLS "" OFF )
option( ASSIMP_INSTALL_PDB "" OFF )
option( ASSIMP_BUILD_SAMPLES "If the official samples are built as well (needs Glut)." OFF )
option( ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." OFF)
add_subdirectory(assimp)

#option(gtest_force_shared_crt OFF)
#option(GTEST_LINKED_AS_SHARED_LIBRARY "" OFF)
#option(BUILD_SHARED_LIBS OFF)
#add_subdirectory(googletest)

# Go back to normal build type
set( CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE_COPY} )
