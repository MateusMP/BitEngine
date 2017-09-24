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
#include "BasicTypes.h"
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

class MyGameSystem : public BitEngine::System
{
public:
	MyGameSystem(BitEngine::GameEngine* ge, BitEngine::CommandSystem* cmd, BitEngine::VideoSystem* vs)
    : System(ge), m_cmdSys(cmd), m_es(ge), m_videoSys(vs)
	{
	}

	const char* getName() const override {
		return "MyGameSystem";
	}

	GameWorld* getWorld(){
		return m_world;
	}

	UserGUI* getGUI(){
		return m_userGUI;
	}

	bool Init() override
	{
		using namespace BitEngine;
		BitEngine::ResourceLoader* loader = getEngine()->getResourceLoader();

		if (!m_es.Init()) {
			return false;
		}
		m_userGUI = new UserGUI(&m_es);
		m_world = new OverWorld(&m_es);

		selfPlayer = new Player("nick_here", 0);
		m_world->AddPlayer(selfPlayer);

		// Commands
		defineCommands();

		// Tests
		const RR<Texture> texture = loader->getResource<BitEngine::Texture>("data/sprites/texture.png");
		const RR<Texture> texture2 = loader->getResource<BitEngine::Texture>("data/sprites/sun.png");
		if (!texture.isValid() || !texture2.isValid())
			return false;

		LOG(GameLog(), BE_LOG_VERBOSE) << "Texture loaded: " << texture->getTextureID();

		//Shader3DSimple::Model* model = modelMng->loadModel<Shader3DSimple>("Models/Rocks_03.fbx");

		RR<Sprite> spr1 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox");
		RR<Sprite> spr2 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_orbit");
		RR<Sprite> spr3 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_piece");

		//BitEngine::SpriteHandle spr1 = loader->getResource<BitEngine::Sprite>("player", BitEngine::Sprite(texture, 128, 128, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1)));
		//BitEngine::SpriteHandle spr2 = sprMng->createSprite("playerOrbit", BitEngine::Sprite(texture, 640, 64, 0.5f, 0.0f, glm::vec4(0, 0, 1.0f, 1.0f)));
		//BitEngine::SpriteHandle spr3 = sprMng->createSprite(BitEngine::Sprite(texture2, 256, 256, 0.5f, 0.5f, glm::vec4(0, 0, 2.0f, 2.0f), true));

		// CREATE PLAYER
		PlayerControl::CreatePlayerTemplate(&m_es);

		// Sparks
		for (int i = 0; i < 9; ++i)
		{
			BitEngine::EntityHandle h = m_es.createEntity();
			BitEngine::ComponentRef<BitEngine::Transform2DComponent> transformComp;
			BitEngine::ComponentRef<BitEngine::Sprite2DComponent> spriteComp;
			BitEngine::ComponentRef<BitEngine::SceneTransform2DComponent> sceneComp;
			BitEngine::ComponentRef<BitEngine::GameLogicComponent> logicComp;
			ADD_COMPONENT_ERROR(transformComp = m_es.AddComponent<BitEngine::Transform2DComponent>(h));
			ADD_COMPONENT_ERROR(spriteComp = m_es.AddComponent<BitEngine::Sprite2DComponent>(h, 6, spr3, BitEngine::Sprite2DRenderer::EFFECT_SPRITE));
			ADD_COMPONENT_ERROR(sceneComp = m_es.AddComponent<BitEngine::SceneTransform2DComponent>(h));
			ADD_COMPONENT_ERROR(logicComp = m_es.AddComponent<BitEngine::GameLogicComponent>(h));
			logicComp->addLogicPiece(new Spinner(getEngine()->getMessenger()));

			transformComp->setLocalPosition(i * 128 + 125, 500);
			spriteComp->alpha = 1.0;
		}

		//check_gl_error();
		
		m_world->Start();
        
        
        //
        
        #ifdef _DEBUG
        getEngine()->getMessenger()->registerListener<BitEngine::CommandSystem::MsgCommandInput>(this);
        #endif
        //shader3Drenderer = new Shader3DProcessor(m_es.t3p);
        sprite2Drenderer = new BitEngine::Sprite2DRenderer(getEngine());
        //getEngine()->getResourceLoader()->
        //m_es->InitComponentProcessor(shader3Drenderer);
        m_es.InitComponentProcessor(sprite2Drenderer);
        
		return true;
	}
	
	#ifdef _DEBUG
	void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
    {
        LOG(GameLog(), BE_LOG_VERBOSE) << "Command: " << msg.commandID;
        if (msg.commandID == RELOAD_SHADERS){
            
            BitEngine::RR<BitEngine::Texture> texture = getEngine()->getResourceLoader()->getResource<BitEngine::Texture>("data/sprites/texture.png");
            getEngine()->getResourceLoader()->reloadResource(texture);
        }
    }
    #endif

	void Shutdown() override
	{
        delete sprite2Drenderer;
	}

	void Update() override
	{
		m_es.Update();

		m_videoSys->getDriver()->clearBufferColor(nullptr, BitEngine::ColorRGBA((1+rand())%255, 0, 0, 0));
        m_videoSys->getDriver()->clearBuffer(nullptr, BitEngine::BufferClearBitMask::COLOR_DEPTH);
        /*
         s hader3Drenderer->setActive*Camera(getWorld()->getActiveCamera());
         shader3Drenderer->Render();
         */
        sprite2Drenderer->setActiveCamera(getGUI()->getCamera());
        sprite2Drenderer->Render();
        
        m_videoSys->updateWindow();
	}

private:
	void defineCommands()
	{
		m_cmdSys->RegisterKeyCommandForAllMods(RIGHT, GAMEPLAY, GLFW_KEY_RIGHT);
		m_cmdSys->RegisterKeyCommandForAllMods(LEFT, GAMEPLAY, GLFW_KEY_LEFT);
		m_cmdSys->RegisterKeyCommandForAllMods(UP, GAMEPLAY, GLFW_KEY_UP);
		m_cmdSys->RegisterKeyCommandForAllMods(DOWN, GAMEPLAY, GLFW_KEY_DOWN);
		m_cmdSys->RegisterMouseCommand(CLICK, GAMEPLAY, GLFW_MOUSE_BUTTON_LEFT, BitEngine::Input::MouseAction::PRESS);
#ifdef _DEBUG
		m_cmdSys->RegisterKeyboardCommand(RELOAD_SHADERS, -1, GLFW_KEY_R, BitEngine::Input::KeyAction::PRESS, BitEngine::Input::KeyMod::CTRL);
#endif
		m_cmdSys->setCommandState(GAMEPLAY);
	}
	
	bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, const BitEngine::Sprite2DComponent* s)
    {
        const float radius = s->sprite->getMaxRadius();
        
        const float kX = matrix[2][0] + radius;
        const float kX_r = matrix[2][0] - radius;
        const float kY = matrix[2][1] + radius;
        const float kY_b = matrix[2][1] - radius;
        
        if (kX < screen.x){
            // printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
            return false;
        }
        if (kX_r > screen.z){
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
            return false;
        }
        if (kY < screen.y){
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
            return false;
        }
        if (kY_b > screen.w){
            //printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
            return false;
        }
        
        return true;
    }

	Player *selfPlayer;
	PlayerController* playerController;

	//

	std::string m_name;

	const BitEngine::Texture *texture;

	BitEngine::CommandSystem* m_cmdSys;
	MyGameEntitySystem m_es;

	UserGUI* m_userGUI;
	GameWorld* m_world; //!< Current active world
	
	//
	BitEngine::VideoSystem* m_videoSys;
    
    //BitEngine::Sprite2DRendererBasic* sprite2Drenderer;
    Shader3DProcessor* shader3Drenderer;
    BitEngine::Sprite2DRenderer* sprite2Drenderer;
};
