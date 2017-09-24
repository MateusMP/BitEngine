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
	Resources/ResourceManager.cpp
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

message(STATUS "BITENGINE_INCLUDE_DIR='${BITENGINE_INCLUDE_DIR}'")

# Create the bitengine lib
add_library(bitengine ${SRCS} ${BITENGINE_INCLUDE_FILES})

if (CMAKE_COMPILER_IS_GNUCC)
	target_link_libraries(bitengine PUBLIC -static-libstdc++)
endif()

if (NEED_GLM)
	message(STATUS "Linking bitengine with glm")
	target_link_libraries(bitengine PUBLIC glm)
endif()
if (NEED_NLOHMANN_JSON)
	message(STATUS "Linking bitengine with nlohmann_json")
	target_link_libraries(bitengine PUBLIC nlohmann_json)
endif()

target_include_directories(bitengine PUBLIC $<BUILD_INTERFACE:${BITENGINE_INCLUDE_DIR}>
											$<INSTALL_INTERFACE:include/>)

install(TARGETS bitengine DESTINATION lib EXPORT bitengineTargets)
