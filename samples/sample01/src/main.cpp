
#include <string>

#include <bitengine/bitengine.h>
#include <bitengine/Core/GeneralTaskManager.h>
#include <bitengine/Core/Resources/DevResourceLoader.h>
#include <bitengine/Platform/opengl/GL2/GL2Driver.h>
#include <bitengine/Platform/glfw/GLFW_Platform.h>

#include "Common/GameGlobal.h"
#include "MyGame.h"

#define GL2_API
#ifdef GL2_API
#include "bitengine/Platform/opengl/GL2/GL2ShaderManager.h"
#include "bitengine/Platform/opengl/GL2/GL2TextureManager.h"
#endif

GAME_UPDATE(gameUpdateTest) {
    return game->update();
}

BitEngine::Logger* GameLog()
{
	static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
	return &log;
}


void setupCommands(BitEngine::CommandSystem* cmdSys) {
	cmdSys->registerKeyCommandForAllMods(RIGHT, GAMEPLAY, GLFW_KEY_RIGHT);
	cmdSys->registerKeyCommandForAllMods(LEFT, GAMEPLAY, GLFW_KEY_LEFT);
	cmdSys->registerKeyCommandForAllMods(UP, GAMEPLAY, GLFW_KEY_UP);
	cmdSys->registerKeyCommandForAllMods(DOWN, GAMEPLAY, GLFW_KEY_DOWN);
	cmdSys->RegisterMouseCommand(CLICK, GAMEPLAY, GLFW_MOUSE_BUTTON_LEFT, BitEngine::Input::MouseAction::PRESS);
#ifdef _DEBUG
	cmdSys->registerKeyboardCommand(RELOAD_SHADERS, -1, GLFW_KEY_R, BitEngine::Input::KeyAction::PRESS, BitEngine::Input::KeyMod::CTRL);
#endif
	cmdSys->setCommandState(GAMEPLAY);
}

class MyGamePlatform : public GLFW_Platform {
	
	public:
	MyGamePlatform(BitEngine::Messenger* m) 
		: GLFW_Platform(m)
	{
		subscribe<RenderEvent>(&MyGamePlatform::onMessage, this);
        subscribe<GLFWWindowClosedMsg>(&MyGamePlatform::onMessage, this);
	}

	void init(BitEngine::EngineConfiguration& eg) {
		GLFW_Platform::init(eg);
	}

    void onMessage(const GLFWWindowClosedMsg& msg) {
        getMessenger()->emit<UserRequestQuitGame>({ GameQuitType::CLOSE_WINDOW });
    }

	void onMessage(const RenderEvent& ev)
	{
		video.getDriver()->clearBufferColor(nullptr, BitEngine::ColorRGBA( (float)((1 + rand()) % 255), 0.f, 0.f, 0.f));
		video.getDriver()->clearBuffer(nullptr, BitEngine::BufferClearBitMask::COLOR_DEPTH);
		
		//ev.state->entitySystem->sh3D.setActiveCamera(ev.state->m_world->getActiveCamera());
		//ev.state->entitySystem->sh3D.Render();
	
		ev.state->entitySystem->spr2D.setActiveCamera(ev.state->m_userGUI->getCamera());
		ev.state->entitySystem->spr2D.Render(video.getDriver());

		video.updateWindow(m_currentWindow);
	}


	void onMessage(const WindowResized& ev)
	{
		if (m_currentWindow != ev.window) {
			glfwMakeContextCurrent(ev.window);
		}

		video.getDriver()->setViewPort(0, 0, ev.width, ev.height);

		if (m_currentWindow != ev.window) {
			glfwMakeContextCurrent(m_currentWindow);
		}
	}
};

void gameExecute(MainMemory& gameMemory) {
    
    // Basic infrastructure
    BitEngine::Messenger messenger;
    BitEngine::EngineConfigurationFileLoader configurations("config.ini");
    BitEngine::GeneralTaskManager taskManager(&messenger);


    BitEngine::EngineConfiguration engineConfig;
    configurations.loadConfigurations(engineConfig);

    // Platform
    MyGamePlatform glfwPlatform(&messenger);
    glfwPlatform.init(engineConfig);

    // Game Specific stuff
    BitEngine::CommandSystem commandSystem(&messenger);

    BitEngine::GL2ShaderManager shaderManager(&taskManager);
    BitEngine::GL2TextureManager textureManager(&taskManager);
    BitEngine::SpriteManager spriteManager;
    gameMemory.shaderManager = &shaderManager;
    gameMemory.textureManager = &textureManager;
    gameMemory.spriteManager = &spriteManager;

    // Setup game state
    gameMemory.messenger = &messenger;
    gameMemory.engineConfig = &engineConfig;
    gameMemory.taskManager = &taskManager;
    
    setupCommands(&commandSystem);

    MyGame game(&gameMemory);

    // TODO: Load game code
    gameMemory.gameUpdate = &gameUpdateTest;

    bool32 running = true;
    while (running) {
        glfwPlatform.input.update();
        glfwPlatform.video.update();

        running = gameMemory.gameUpdate(&game);

        messenger.dispatch();
    }

    taskManager.shutdown();
}

int main(int argc, const char* argv[])
{
    BitEngine::LoggerSetup::Setup(argc, argv);
    LOG_FUNCTION_TIME(GameLog());

    MainMemory gameMemory = {};
    gameMemory.memorySize = MEGABYTES(512);
    gameMemory.memory = malloc(gameMemory.memorySize);
    memset(gameMemory.memory, 0, gameMemory.memorySize);
    
    gameExecute(gameMemory);

	free(gameMemory.memory);

    return 0;
}
