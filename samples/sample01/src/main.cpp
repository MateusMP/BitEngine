
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
    cmdSys->RegisterMouseCommand(CLICK, GAMEPLAY, BE_MOUSE_BUTTON_LEFT, BitEngine::MouseAction::PRESS);
#ifdef _DEBUG
    cmdSys->registerKeyboardCommand(RELOAD_SHADERS, -1, BE_KEY_R, BitEngine::KeyAction::PRESS, BitEngine::KeyMod::CTRL);
#endif
    cmdSys->setCommandState(GAMEPLAY);
}

void resourceManagerMenu(const char* name, BitEngine::ResourceManager *resMng) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "RAM: %.2f GPU: %.2f", resMng->getCurrentRamUsage()* TO_MB, resMng->getCurrentGPUMemoryUsage()*TO_MB);
        ImGui::TreePop();
    }
}

void gameExecute(MainMemory& gameMemory) {

    // Basic infrastructure
    BitEngine::EngineConfigurationFileLoader configurations("config.ini");
    BitEngine::GeneralTaskManager taskManager;

    BitEngine::EngineConfiguration engineConfig;
    configurations.loadConfigurations(engineConfig);

    BitEngine::GLFW_VideoSystem video;
    BitEngine::GLFW_ImGuiSystem imgui;
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

    BitEngine::GLFW_InputSystem input;
    input.registerWindow(main_window);


    // Game Specific stuff
    BitEngine::CommandSystem commandSystem(main_window);

    BitEngine::GL2ShaderManager shaderManager(&taskManager);
    BitEngine::GL2TextureManager textureManager(&taskManager);
    BitEngine::SpriteManager spriteManager;

    BitEngine::DevResourceLoader loader(&taskManager);
    loader.registerResourceManager<BitEngine::Shader>("SHADER", &shaderManager);
    loader.registerResourceManager<BitEngine::Texture>("TEXTURE", &textureManager);
    loader.registerResourceManager<BitEngine::Sprite>("SPRITE", &spriteManager);
    loader.init();

    gameMemory.loader = &loader;
    gameMemory.videoSystem = &video;
    gameMemory.window = main_window;
    gameMemory.engineConfig = &engineConfig;
    gameMemory.taskManager = &taskManager;
    gameMemory.commandSystem = &commandSystem;
    gameMemory.imGuiRender = &imgui;
    gameMemory.logger = GameLog();

    auto imguiMenu = [&](const BitEngine::ImGuiRenderEvent& event) {
        resourceManagerMenu("Sprite Manager", &spriteManager);
        resourceManagerMenu("Texture Manager", &textureManager);
        resourceManagerMenu("Shader Manager", &shaderManager);
    };
    imgui.subscribe(imguiMenu);

    setupCommands(&commandSystem);

    {
        MyGame* game = new MyGame(&gameMemory);

        bool32 running = true;

        while (running) {
            input.update();

            main_window->drawBegin();

            running = game->update();

            imgui.update();

            main_window->drawEnd();
        }
        delete game;
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
