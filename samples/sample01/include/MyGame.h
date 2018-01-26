#pragma once

#include "Overworld.h"

#include "MyGameSystem.h"

class MyGame : public BitEngine::MessengerEndpoint
{
	public:
	MyGame(GameMemory* gameMemory)
		: MessengerEndpoint(gameMemory->messenger), gameMemory(gameMemory)
	{
		subscribe<BitEngine::Input::MsgKeyboardInput>(&MyGame::onMessage, this);
		subscribe<BitEngine::CommandSystem::MsgCommandInput>(&MyGame::onMessage, this);

		gameState = (GameState*)gameMemory->memory;
	}

	~MyGame()
	{
	}

	bool init()
	{
		using namespace BitEngine;
		BitEngine::ResourceLoader* loader = gameMemory->resources;

		BE_ASSERT(loader->hasManagerForType("SHADER"));
		BE_ASSERT(loader->hasManagerForType("SPRITE"));
		BE_ASSERT(loader->hasManagerForType("TEXTURE"));
		
		MemoryArena& permanentArena = gameState->permanentArena;

		// Alloc memory
		permanentArena.init((u8*)gameMemory->memory + sizeof(GameState), MEGABYTES(8));
		gameState->entityArena.init(gameState->permanentArena.endPtr(), MEGABYTES(64));
			
		// Init game state stuff
		gameState->entitySystem = permanentArena.push<MyGameEntitySystem>(loader, gameMemory->messenger, &gameState->entityArena);
		gameState->entitySystem->Init();

		gameState->m_userGUI = permanentArena.push<UserGUI>(gameState->entitySystem);
		gameState->m_world = permanentArena.push<OverWorld>(gameState->entitySystem);

		gameState->selfPlayer = permanentArena.push<Player>("nick_here", 0);
		gameState->m_world->AddPlayer(gameState->selfPlayer);
		
		// Tests
		const RR<Texture> texture = loader->getResource<BitEngine::Texture>("data/sprites/texture.png");
		const RR<Texture> texture2 = loader->getResource<BitEngine::Texture>("data/sprites/sun.png");
		if (!texture.isValid() || !texture2.isValid()) {
			return false;
		}

		LOG(GameLog(), BE_LOG_VERBOSE) << "Texture loaded: " << texture->getTextureID();

		//Shader3DSimple::Model* model = modelMng->loadModel<Shader3DSimple>("Models/Rocks_03.fbx");

		RR<Sprite> spr1 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox");
		RR<Sprite> spr2 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_orbit");
		RR<Sprite> spr3 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_piece");

		//BitEngine::SpriteHandle spr1 = loader->getResource<BitEngine::Sprite>("player", BitEngine::Sprite(texture, 128, 128, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1)));
		//BitEngine::SpriteHandle spr2 = sprMng->createSprite("playerOrbit", BitEngine::Sprite(texture, 640, 64, 0.5f, 0.0f, glm::vec4(0, 0, 1.0f, 1.0f)));
		//BitEngine::SpriteHandle spr3 = sprMng->createSprite(BitEngine::Sprite(texture2, 256, 256, 0.5f, 0.5f, glm::vec4(0, 0, 2.0f, 2.0f), true));

		// CREATE PLAYER
		PlayerControl::CreatePlayerTemplate(loader, gameState->entitySystem);

		// Sparks
		EntitySystem* es = gameState->entitySystem;
		for (int i = 0; i < 9; ++i)
		{
			BitEngine::EntityHandle h = gameState->entitySystem->createEntity();
			BitEngine::ComponentRef<BitEngine::Transform2DComponent> transformComp;
			BitEngine::ComponentRef<BitEngine::Sprite2DComponent> spriteComp;
			BitEngine::ComponentRef<BitEngine::SceneTransform2DComponent> sceneComp;
			BitEngine::ComponentRef<BitEngine::GameLogicComponent> logicComp;
			ADD_COMPONENT_ERROR(transformComp = es->AddComponent<BitEngine::Transform2DComponent>(h));
			ADD_COMPONENT_ERROR(spriteComp = es->AddComponent<BitEngine::Sprite2DComponent>(h, 6, spr3, BitEngine::Sprite2DRenderer::EFFECT_SPRITE));
			ADD_COMPONENT_ERROR(sceneComp = es->AddComponent<BitEngine::SceneTransform2DComponent>(h));
			ADD_COMPONENT_ERROR(logicComp = es->AddComponent<BitEngine::GameLogicComponent>(h));
			logicComp->addLogicPiece(new Spinner(getMessenger()));

			transformComp->setLocalPosition(i * 128 + 125, 500);
			spriteComp->alpha = 1.0;
		}

		//check_gl_error();

		gameState->m_world->Start();

		return true;
	}

	void update()
	{
		if (!gameState->initialized) {
			// Init stuff
			init();
			gameState->initialized = true;
		}
		
		gameState->entitySystem->Update();

		gameMemory->taskManager->update();

		// Render
			
		getMessenger()->emit<RenderEvent>(RenderEvent{ gameState });
	}

	void onMessage(const BitEngine::Input::MsgKeyboardInput& msg)
	{
		if (msg.key == GLFW_KEY_R && msg.keyAction == BitEngine::Input::KeyAction::PRESS && msg.keyMod == BitEngine::Input::KeyMod::CTRL_SHIFT)
		{
			LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Reloading index";
			gameMemory->resources->loadIndex("data/main.idx");
		}
	}

	void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
	{
		LOG(GameLog(), BE_LOG_VERBOSE) << "Command: " << msg.commandID;
		if (msg.commandID == RELOAD_SHADERS) {

			BitEngine::RR<BitEngine::Texture> texture = gameMemory->resources->getResource<BitEngine::Texture>("data/sprites/texture.png");
			gameMemory->resources->reloadResource(texture);
			BitEngine::RR<BitEngine::Texture> texture2 = gameMemory->resources->getResource<BitEngine::Texture>("data/sprites/sun.png");
			gameMemory->resources->reloadResource(texture2);
		}
	}

	void onMessage(const BitEngine::MsgFrameStart& msg)
	{
		//LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Frame Start";
	}

	private:
	GameMemory* gameMemory;
	GameState* gameState;

};



static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, const BitEngine::Sprite2DComponent* s)
{
	const float radius = s->sprite->getMaxRadius();

	const float kX = matrix[2][0] + radius;
	const float kX_r = matrix[2][0] - radius;
	const float kY = matrix[2][1] + radius;
	const float kY_b = matrix[2][1] - radius;

	if (kX < screen.x) {
		// printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
		return false;
	}
	if (kX_r > screen.z) {
		//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
		return false;
	}
	if (kY < screen.y) {
		//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
		return false;
	}
	if (kY_b > screen.w) {
		//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
		return false;
	}

	return true;
}