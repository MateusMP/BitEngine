
#include <string>

#include <bitengine/bitengine.h>
#include <bitengine/Core/GameEnginePC.h>
#include <bitengine/Core/Resources/DevResourceLoader.h>
#include <bitengine/DefaultBackends/opengl/GL2/GL2Driver.h>
#include <bitengine/DefaultBackends/glfw/GLFW_Platform.h>

#include "Common/GameGlobal.h"
#include "MyGame.h"

#define GL2_API
#ifdef GL2_API
#include "bitengine/DefaultBackends/opengl/GL2/GL2ShaderManager.h"
#include "bitengine/DefaultBackends/opengl/GL2/GL2TextureManager.h"
#endif

BitEngine::Logger* GameLog()
{
	static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
	return &log;
}

//
static GAME_UPDATE(gameUpdateTest) 
{
	static MyGame game(gameMemory);

	game.update();

	return true;
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
	}

	void init(BitEngine::EngineConfiguration& eg) {
		GLFW_Platform::init(eg);
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


class UpdateTask : public BitEngine::Task
{
	public:
	UpdateTask(std::function<void()> s)
		: Task(Task::TaskMode::REPEAT_ONCE_PER_FRAME_REQUIRED, Task::Affinity::MAIN), f(s) {}

	void run()
	{
		f();
	}

	bool finished()
	{
		return false;
	}

	private:
	std::function<void()> f;
};

int main()
{
	LOG_FUNCTION_TIME(GameLog());

	GameMemory gameMemory = {};
	gameMemory.memorySize = MEGABYTES(256);
	gameMemory.memory = malloc(gameMemory.memorySize);
	memset(gameMemory.memory, 0, gameMemory.memorySize);
	
	// Basic infrastructure
	BitEngine::Messenger messenger;
	BitEngine::EngineConfigurationFileLoader configurations("config.ini");
	BitEngine::GeneralTaskManager taskManager(&messenger);
	BitEngine::DevResourceLoader resourceLoader(&messenger, &taskManager);
		
	BitEngine::EngineConfiguration engineConfig;
	configurations.loadConfigurations(engineConfig);
	
	// Platform
	MyGamePlatform glfwPlatform(&messenger);
	glfwPlatform.init(engineConfig);

	// Game Specific stuff
	BitEngine::CommandSystem commandSystem(&messenger);

	BitEngine::SpriteManager spriteManager;
	BitEngine::GL2ShaderManager shaderManager(&taskManager);
	BitEngine::GL2TextureManager textureManager(&taskManager);

	resourceLoader.registerResourceManager("SPRITE", &spriteManager);
	resourceLoader.registerResourceManager("SHADER", &shaderManager);
	resourceLoader.registerResourceManager("TEXTURE", &textureManager);
	resourceLoader.init();
	
	// Setup game state
	gameMemory.messenger = &messenger;
	gameMemory.resources = &resourceLoader;
	gameMemory.engineConfig = &engineConfig;
	gameMemory.taskManager = &taskManager;
		
	// TODO: Load game code
	gameMemory.gameUpdate = &gameUpdateTest;

	setupCommands(&commandSystem);
	
	// Init resources
	resourceLoader.loadIndex("data/main.idx");


	taskManager.addTask(std::make_shared<UpdateTask>([&resourceLoader] {resourceLoader.update(); }));
	
	bool32 running = true;
	while (running) {
		glfwPlatform.input.update();

		running = gameMemory.gameUpdate(&gameMemory);
		messenger.dispatch();
	}

	free(gameMemory.memory);

    return 0;
}
