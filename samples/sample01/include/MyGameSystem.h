#pragma once

#include <bitengine/Core/System.h>
#include <bitengine/Core/VideoSystem.h>
#include <bitengine/Core/GameEngine.h>
#include <bitengine/Core/Graphics/Sprite2D.h>
#include <bitengine/Core/ECS/EntitySystem.h>

#include "Graphics/Shader3DSimple.h"
#include "MyGameEntitySystem.h"
#include "Overworld.h"

#include "PlayerControlLogic.h"

#include "Graphics/Shader3DSimple.h"
#include "Graphics/Shader3DProcessor.h"

class Spinner : public BitEngine::GameLogic
{
public:
	Spinner(BitEngine::Messenger* m)
		: GameLogic(m)
	{}

	RunEvents getRunEvents() override {
		return RunEvents::EALL;
	}

	bool init() override {
		transform = getComponent<BitEngine::Transform2DComponent>();
		if (!transform.isValid()){
			return false;
		}

		k = (rand() % 10) / 100.0f + 0.02f;

		return true;
	}

	void frameStart() override {

	};
	void frameMiddle() override {
		transform->setLocalRotation(transform->getLocalRotation() + k);
	};
	void frameEnd() override {

	};

	void end() override {
	}

private:
	BitEngine::ComponentRef<BitEngine::Transform2DComponent> transform;
	float k;
};

class UserGUI
{
public:
	UserGUI(MyGameEntitySystem* es)
	{
		gui = es->createEntity();

		camera = es->AddComponent<BitEngine::Camera2DComponent>(gui);

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
