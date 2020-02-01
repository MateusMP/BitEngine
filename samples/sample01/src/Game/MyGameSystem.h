#pragma once

#include <BitEngine/Core/VideoSystem.h>
#include <BitEngine/Core/Graphics/Sprite2D.h>
#include <BitEngine/Core/ECS/EntitySystem.h>

#include "MyGameEntitySystem.h"
#include "Overworld.h"

class UserGUI
{
public:
	UserGUI(MyGameEntitySystem* es)
	{
		gui = es->createEntity();

		camera = es->addComponent<BitEngine::Camera2DComponent>(gui);

		camera->setView(1280, 720);
		camera->setLookAt(glm::vec3(1280/2, 720/2, 0));
		camera->setZoom(1.0f);
	}

	BitEngine::ComponentRef<BitEngine::Camera2DComponent>& getCamera() {
		return camera;
	}

private:
	BitEngine::EntityHandle gui;
	BitEngine::ComponentRef<BitEngine::Camera2DComponent> camera;
};
