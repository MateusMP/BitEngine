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
    SpriteManager.cpp
    Timer.cpp
    VideoRenderer.cpp
)
add_subdirectory(ECS)
add_subdirectory(Graphics)
add_subdirectory(Resources)

add_library(be_core STATIC 
					${SRCS}
					$<TARGET_OBJECTS:be_ecs>
					$<TARGET_OBJECTS:be_graphics>
					$<TARGET_OBJECTS:be_resources>)