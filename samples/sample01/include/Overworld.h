#pragma once

#include <bitengine/Core/GameEngine.h>

#include "Common/GameGlobal.h"
#include "GameWorld.h"
#include "Player.h"

#include "BasicTypes.h"

class PlayerCamera
{
public:
	virtual bool Create(MyGameEntitySystem* esys)
	{
		entity = esys->createEntity();
		transform = esys->AddComponent<BitEngine::Transform3DComponent>(entity);
		camera = esys->AddComponent<BitEngine::Camera3DComponent>(entity);

		transform->setPosition(0, 0, 250);

		if (!transform.isValid()) return false;
		if (!camera.isValid()) return false;

		return true;
	}

	void setLookAt(const glm::vec3& pos){
		camera->setLookAt(pos);
	}

	BitEngine::ComponentRef<BitEngine::Camera3DComponent>& getCamera() {
		return camera;
	}

	BitEngine::ComponentRef<BitEngine::Transform3DComponent>& getTransform() {
		return transform;
	}

protected:
	BitEngine::EntityHandle entity;
	BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;
	BitEngine::ComponentRef<BitEngine::Camera3DComponent> camera;
};

class PlayerController : public BitEngine::GameLogic
{
public:
	PlayerController(BitEngine::Messenger*m, Player* p, PlayerCamera* cam)
		: GameLogic(m), player(p), camera(cam)
	{}

	RunEvents getRunEvents() override {
		return RunEvents::EALL;
	}

	bool init() override
	{
		transform = getComponent<BitEngine::Transform3DComponent>();
		if (!transform.isValid()){
			return false;
		}

		getMessenger()->registerListener<BitEngine::CommandSystem::MsgCommandInput>(this);

		return true;
	}

	void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
	{
		switch (msg.commandID){
            case RIGHT:
                moveDirection.x = 1.0f;
                if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
                    moveDirection.x = 0.0f;
                break;
            case LEFT:
                moveDirection.x = -1.0f;
                if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
                    moveDirection.x = 0.0f;
                break;
            case UP:
                moveDirection.z = -1.0f;
                if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
                    moveDirection.z = 0.0f;
                break;
            case DOWN:
                moveDirection.z = 1.0f;
                if (msg.action.fromButton == BitEngine::Input::KeyAction::RELEASE)
                    moveDirection.z = 0.0f;
                break;
		}

		if (glm::length(moveDirection) > 0)
			moveDirection = glm::normalize(moveDirection);
	}

	void frameStart() override {
		const auto ctransform = transform;

		glm::vec3 velocity(moveDirection.x, 0, moveDirection.z);

		transform->setPosition(ctransform->getPosition() + velocity);
		playerLookDirection = ctransform->getPosition() + BitEngine::Transform3DComponent::FORWARD;

		camera->setLookAt(playerLookDirection);
	};

	void frameMiddle() override {

	};

	void frameEnd() override {

	};

	void end() override {
	}

	PlayerCamera* getCamera() const {
		return camera;
	}

	const glm::vec3& getPlayerLookDirection() const {
		return playerLookDirection;
	}

private:
	Player* player;
	PlayerCamera* camera;
	BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;

	float maxSpeed = 10;
	glm::vec3 moveDirection;
	glm::vec3 playerLookDirection;
};

class OverWorld : public GameWorld
{
public:
	OverWorld(MyGameEntitySystem* es)
		: GameWorld(es) {}

	bool Init() override {
		return true;
	}

	void Shutdown() override {
	}

	int AddPlayer(Player* player) override
	{
		m_players.push_back(player);

		return true;
	}

	void Start() override
	{
		createCharacters();

	}


private:
	void createCharacters()
	{
		BitEngine::ComponentRef<BitEngine::GameLogicComponent> plCtrlComp;

		// Create SELF PLAYER
		Player* player = m_players[0];
		OverworldCharacter* character = new OverworldCharacter(player->getNetID());
		character->Create(m_ES);

		PlayerCamera* playerCamera = new PlayerCamera();
		if (!playerCamera->Create(m_ES)){
			LOG(GameLog(), BE_LOG_ERROR) << "FAILED TO CREATE PlayerCamera";
		}

		m_ES->t3p->setParentOf(playerCamera->getTransform(), character->getTransform());

		PlayerController* playerController = new PlayerController(m_ES->getEngine()->getMessenger(), player, playerCamera);

		plCtrlComp = m_ES->AddComponent<BitEngine::GameLogicComponent>(character->getEntity());
		plCtrlComp->addLogicPiece(playerController);
		player->setCharacter(character);

		setActiveCamera(playerCamera->getCamera());

		// Create other players
		for (u32 i = 1; i < m_players.size(); ++i)
		{
			player = m_players[i];
			character = new OverworldCharacter(player->getNetID());
			character->Create(m_ES);
			player->setCharacter(character);
		}
	}


	std::vector<Player*> m_players;

	std::vector<OverworldCharacter*> characters;
};
