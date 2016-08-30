set(SRCS
    Input.cpp
    InputSystem.cpp
    Logger.cpp
    Messenger.cpp
    ModelManager.cpp
    ResourceSystem.cpp
    SpriteManager.cpp
    Timer.cpp
    VideoRenderer.cpp
    VideoSystem.cpp
)
add_subdirectory(ECS)
add_subdirectory(Graphics)
add_subdirectory(Resources)

add_library(core ${SRCS})