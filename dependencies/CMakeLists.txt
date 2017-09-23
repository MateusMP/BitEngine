cmake_minimum_required(VERSION 3.1.0)

include (ExternalProject)

project(Build3rdParty CXX)

include("nlohmann_json.cmake")
include("glm.cmake")
include("stb.cmake")
include("glew.cmake")
include("glfw.cmake")
include("gtest.cmake")
