#pragma once

#include <bitengine\bitengine.h>

#include "MyGameEntitySystem.h"

class PlayerControl : public BitEngine::GameLogic
{
public:
	static BitEngine::EntityHandle CreatePlayerTemplate(BitEngine::ResourceLoader* loader, MyGameEntitySystem* es)
	{
		using namespace BitEngine;

		RR<Sprite> playerSPR = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox");
		RR<Sprite> playerOrbitSPR = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_orbit");

		EntityHandle ent_player;
		ComponentRef<Transform2DComponent> playerT2D;
		ComponentRef<SceneTransform2DComponent> playerST2D;
		ComponentRef<Sprite2DComponent> playerSpr2D;
		ComponentRef<GameLogicComponent> gamelogic;

		ent_player = es->createEntity();
		LOG(GameLog(), BE_LOG_VERBOSE) << "ent_player: " << ent_player;

		// 2D
		ADD_COMPONENT_ERROR(playerT2D = es->AddComponent<Transform2DComponent>(ent_player));
		ADD_COMPONENT_ERROR(playerST2D = es->AddComponent<SceneTransform2DComponent>(ent_player));
		ADD_COMPONENT_ERROR(playerSpr2D = es->AddComponent<Sprite2DComponent>(ent_player));
		ADD_COMPONENT_ERROR(gamelogic = es->AddComponent<GameLogicComponent>(ent_player));
		playerT2D->setLocalPosition(128, 128);
		playerSpr2D->layer = 5;
		playerSpr2D->sprite = playerSPR;
		gamelogic->addLogicPiece(new PlayerControl(es->getMessenger()));

		// 2D orbit
		ComponentRef<Transform2DComponent> pcT;
		ComponentRef<Sprite2DComponent> pcS;
		ComponentRef<SceneTransform2DComponent> pcST;
		EntityHandle playerConnected = es->createEntity();
		ADD_COMPONENT_ERROR(pcT = es->AddComponent<Transform2DComponent>(playerConnected));
		ADD_COMPONENT_ERROR(pcST = es->AddComponent<SceneTransform2DComponent>(playerConnected));
		ADD_COMPONENT_ERROR(pcS = es->AddComponent<Sprite2DComponent>(playerConnected));
		es->t2p.setParentOf(pcT, playerT2D);
		pcT->setLocalPosition(128, 128);
		pcT->setLocalRotation(45 * 3.1415f / 180.0f);
		pcS->layer = 6;
		pcS->sprite = playerOrbitSPR;


		// PLAYER 3D
		/*Shader3DSimple::Model* model = modelMng->loadModel<Shader3DSimple>("Models/Rocks_03.fbx");

		BitEngine::ComponentRef<BitEngine::Transform3DComponent> playerT3D;
		BitEngine::ComponentRef<BitEngine::RenderableMeshComponent> meshRef;
		ADD_COMPONENT_ERROR(playerT3D = es->AddComponent<BitEngine::Transform3DComponent>(ent_player));
		playerT3D->setPosition(0, 0, 0);
		ADD_COMPONENT_ERROR(meshRef = es->AddComponent<BitEngine::RenderableMeshComponent>(ent_player));
		meshRef->setMesh(model->getMesh(0));*/

		return ent_player;
	}

	PlayerControl(BitEngine::Messenger* m) : GameLogic(m){}

	RunEvents getRunEvents() override
	{
		return RunEvents::EALL;
	}

	void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
	{
		switch (msg.commandID)
		{
		case RIGHT:
			movH = msg.intensity;
			if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
				movH = 0;
			break;
		case LEFT:
			movH = -msg.intensity;
			if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
				movH = 0;
			break;
		case UP:
			movV = msg.intensity;
			if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
				movV = 0;
			break;
		case DOWN:
			movV = -msg.intensity;
			if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
				movV = 0;
			break;

		case CLICK:
			printf("CLICK!!!!\n\n");
			break;
		}
	}

	bool init() override
	{
		getMessenger()->subscribe<BitEngine::CommandSystem::MsgCommandInput>(&PlayerControl::onMessage, this);

		movH = movV = 0.0f;

		transform2d = getComponent<BitEngine::Transform2DComponent>();
		transform2d->setLocalPosition(0, 0);

		return true;
	}

	void frameStart() override
	{
		float vel = 2.0f;

		// camT2D->setPosition(x, y);
		glm::vec2 pos = transform2d->getLocalPosition();
		pos.x += movH * vel;
		pos.y += movV * vel;

		transform2d->setLocalPosition(pos);
		transform2d->setLocalRotation(transform2d->getLocalRotation() + 0.03f);
	}

	void frameMiddle() override
	{

	}

	void frameEnd() override
	{

	}

	void end() override
	{

	}

private:
	float movH, movV;
	float x, y;

	BitEngine::ComponentRef<BitEngine::Transform2DComponent> transform2d;
};
