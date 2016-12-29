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
add_library(be_core OBJECT ${SRCS})

add_subdirectory(ECS)
add_subdirectory(Graphics)
add_subdirectory(Resources)