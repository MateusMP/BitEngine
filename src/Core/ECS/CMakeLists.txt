set(SRCS
	BaseEntitySystem.cpp
	Camera2DComponent.cpp
	Camera2DProcessor.cpp
	Camera3DComponent.cpp
	Camera3DProcessor.cpp
	ComponentProcessor.cpp
	GameLogicProcessor.cpp
	RenderableMeshProcessor.cpp
	Transform2DComponent.cpp
	Transform2DProcessor.cpp
	Transform3DComponent.cpp
	Transform3DProcessor.cpp
)

add_library(be_ecs OBJECT ${SRCS})