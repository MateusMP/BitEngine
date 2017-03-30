# root
set(SRCS
	CommandSystem.cpp
	EngineConfiguration.cpp
	GameEnginePC.cpp
	GeneralTaskManager.cpp
    Input.cpp
	Logger.cpp
    ModelManager.cpp
	Reflection.cpp
    ResourceSystem.cpp
    Timer.cpp
    VideoRenderer.cpp	
    
)
# Resources
set(SRCS ${SRCS}
	Resources/DevResourceLoader.cpp
	Resources/SpriteManager.cpp
)
# Graphics
set(SRCS ${SRCS}
	Graphics/Sprite2D.cpp
)

set(SRCS ${SRCS}
	ECS/BaseEntitySystem.cpp
	ECS/Camera2DComponent.cpp
	ECS/Camera2DProcessor.cpp
	ECS/Camera3DComponent.cpp
	ECS/Camera3DProcessor.cpp
	ECS/ComponentProcessor.cpp
	ECS/GameLogicProcessor.cpp
	ECS/RenderableMeshProcessor.cpp
	ECS/Transform2DComponent.cpp
	ECS/Transform2DProcessor.cpp
	ECS/Transform3DComponent.cpp
	ECS/Transform3DProcessor.cpp
)

message(STATUS "BE_INCLUDE_DIR='${BE_INCLUDE_DIR}'")
message(STATUS "BE_INCLUDE_FILES='${BE_INCLUDE_FILES}'")
message(STATUS "ALL_BE_INCLUDES='${ALL_BE_INCLUDES}'")

# Find header files
FILE(GLOB_RECURSE BE_INCLUDE_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../../include/*.h")

# Create header custom project so that they show up correctly on generated projects
add_custom_target(headers SOURCES ${BE_INCLUDE_FILES})

# Create the bitengine lib
add_library(bitengine ${SRCS} ${BE_INCLUDE_FILES})
target_compile_options(bitengine PUBLIC -static-libstdc++)
if (NEED_GLM)
	message(STATUS "Linking bitengine with glm")
	#add_dependencies(bitengine glm_dep)
	target_link_libraries(bitengine PUBLIC glm)
endif()
if (NEED_NLOHMANN_JSON)
	message(STATUS "Linking bitengine with nlohmann_json")
	#add_dependencies(bitengine nlohmann_json_dep)
	target_link_libraries(bitengine PUBLIC nlohmann_json)
endif()

target_include_directories(bitengine PUBLIC $<BUILD_INTERFACE:${BE_INCLUDE_DIR}>
											$<INSTALL_INTERFACE:include/>)
target_include_directories(bitengine PRIVATE ${ALL_BE_INCLUDES} ${JSON_INCLUDE_DIR})

install(TARGETS bitengine DESTINATION lib EXPORT bitengineTargets)
