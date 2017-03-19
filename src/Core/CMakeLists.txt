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

FILE(GLOB_RECURSE BE_INCLUDE_FILES ${PROJECT_SOURCE_DIR}/include/*.h)
add_custom_target(headers SOURCES ${BE_INCLUDE_FILES})

add_library(bitengine ${SRCS} ${BE_INCLUDE_FILES})
target_include_directories(bitengine PRIVATE ${ALL_BE_INCLUDES} ${BE_INCLUDE})

install(TARGETS bitengine DESTINATION ${LIB_INSTALL_DIR} EXPORT bitengine-targets)
