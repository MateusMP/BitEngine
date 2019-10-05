
#include <string>

#include <bitengine/bitengine.h>
#include <bitengine/Core/Messenger.h>
#include <bitengine/Core/GeneralTaskManager.h>
#include <bitengine/Core/Resources/DevResourceLoader.h>
#include <Platform/opengl/GL2/GL2Driver.h>

#include "Common/GameGlobal.h"
#include "MyGame.h"

#define GL2_API
#ifdef GL2_API
#include "Platform/opengl/GL2/GL2ShaderManager.h"
#include "Platform/opengl/GL2/GL2TextureManager.h"
#endif

#include "Platform/GLFW/GLFW_VideoSystem.h"
#include "Platform/GLFW/GLFW_InputSystem.h"
#include "Platform/GLFW/GLFW_ImGuiSystem.h"

#include "imgui.h"

GAME_UPDATE(gameUpdateTest) {
    return game->update();
}

BitEngine::Logger* GameLog()
{
    static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
    return &log;
}


void setupCommands(BitEngine::CommandSystem* cmdSys) {
    cmdSys->registerKeyCommandForAllMods(RIGHT, GAMEPLAY, BE_KEY_RIGHT);
    cmdSys->registerKeyCommandForAllMods(LEFT, GAMEPLAY, BE_KEY_LEFT);
    cmdSys->registerKeyCommandForAllMods(UP, GAMEPLAY, BE_KEY_UP);
    cmdSys->registerKeyCommandForAllMods(DOWN, GAMEPLAY, BE_KEY_DOWN);
    cmdSys->RegisterMouseCommand(CLICK, GAMEPLAY, BE_MOUSE_BUTTON_LEFT, BitEngine::Input::MouseAction::PRESS);
#ifdef _DEBUG
    cmdSys->registerKeyboardCommand(RELOAD_SHADERS, -1, BE_KEY_R, BitEngine::Input::KeyAction::PRESS, BitEngine::Input::KeyMod::CTRL);
#endif
    cmdSys->setCommandState(GAMEPLAY);
}


void gameExecute(MainMemory& gameMemory) {

    // Basic infrastructure
    BitEngine::Messenger messenger;
    BitEngine::EngineConfigurationFileLoader configurations("config.ini");
    BitEngine::GeneralTaskManager taskManager(&messenger);

    BitEngine::EngineConfiguration engineConfig;
    configurations.loadConfigurations(engineConfig);

    BitEngine::GLFW_VideoSystem video(&messenger);
    BitEngine::GLFW_ImGuiSystem imgui(&messenger);
    video.init();

    BitEngine::Window *main_window;

    BitEngine::WindowConfiguration windowConfig;
    windowConfig.m_Title = "WINDOW";
    windowConfig.m_Width = (u32)engineConfig.getConfiguration("Video", "Width", "1280")->getValueAsReal();
    windowConfig.m_Height = (u32)engineConfig.getConfiguration("Video", "Height", "720")->getValueAsReal();
    windowConfig.m_Resizable = engineConfig.getConfiguration("Video", "Resizable", "true")->getValueAsBool();
    windowConfig.m_FullScreen = engineConfig.getConfiguration("Video", "Fullscreen", "false")->getValueAsBool();

    windowConfig.m_RedBits = 8;
    windowConfig.m_GreenBits = 8;
    windowConfig.m_BlueBits = 8;
    windowConfig.m_AlphaBits = 8;

    windowConfig.m_DepthBits = 8;
    windowConfig.m_StencilBits = 8;
    main_window = video.createWindow(windowConfig);
    imgui.setup(main_window);

    BitEngine::GLFW_InputSystem input(&messenger);
    input.registerWindow(main_window);

    // Game Specific stuff
    BitEngine::CommandSystem commandSystem(&messenger);

    BitEngine::GL2ShaderManager shaderManager(&taskManager);
    BitEngine::GL2TextureManager textureManager(&taskManager);
    BitEngine::SpriteManager spriteManager;
    gameMemory.shaderManager = &shaderManager;
    gameMemory.textureManager = &textureManager;
    gameMemory.spriteManager = &spriteManager;
    gameMemory.videoSystem = &video;

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
        input.update();

        main_window->drawBegin();

        running = gameMemory.gameUpdate(&game);

        imgui.update();

        main_window->drawEnd();

        messenger.dispatch();
    }

    taskManager.shutdown();

    input.shutdown();
    video.shutdown();
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
