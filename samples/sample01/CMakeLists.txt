# Dependencies
set(NEED_ASSIMP 1)

# PROJECT

set(CPP_FILES src/main.cpp
			  src/Graphics/Shader3DBatchRenderer.cpp
			  src/Graphics/Shader3DSimple.cpp)
message(STATUS "ASSIMP LIB='${ASSIMP_LIBRARY}'")

add_executable(SAMPLE_01 ${CPP_FILES})
#target_compile_options(SAMPLE_01 PUBLIC -static-libstdc++)

if (CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(SAMPLE_01 PUBLIC -static-libstdc++)
endif()

target_link_libraries(SAMPLE_01 PUBLIC bitengine backend-glfw backend-gl2 assimp)
target_include_directories(SAMPLE_01 PUBLIC ${CMAKE_SOURCE_DIR}/dependencies/assimp/include ${CMAKE_BINARY_DIR}/dependencies/assimp/include)
target_include_directories(SAMPLE_01 PUBLIC include/)

#install(FILES assimp DESTINATION sample01)
set_target_properties(SAMPLE_01 PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

install(DIRECTORY data DESTINATION sample01)
install(TARGETS SAMPLE_01 DESTINATION sample01)
