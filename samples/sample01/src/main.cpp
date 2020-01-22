
#include <string>

#include <bitengine/bitengine.h>
#include <bitengine/Core/Messenger.h>
#include <bitengine/Core/GeneralTaskManager.h>
#include <bitengine/Core/Resources/DevResourceLoader.h>

#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"
#include "Game/Common/CommonMain.h"
#include "Game/MyGame.h"

// OPEN GL
#include <Platform/opengl/GL2/GL2ShaderManager.h>
#include <Platform/opengl/GL2/GL2TextureManager.h>
#include <Platform/GLFW/GLFW_VideoSystem.h>
#include <Platform/GLFW/GLFW_InputSystem.h>
#include <Platform/GLFW/GLFW_ImGuiSystem.h>

#include "GamePlatform/OpenGL/OpenGLRenderer.h"


BitEngine::Logger* GameLog()
{
    static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
    return &log;
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

    // Setup resource loader
    const u32 resMemSize = MEGABYTES(64);
    void* resMem = malloc(resMemSize);
    BitEngine::MemoryArena resourceArena;
    memset(resMem, 0, resMemSize);
    resourceArena.init((u8*)resMem, resMemSize);

    BitEngine::DevResourceLoader loader(&taskManager, resourceArena);
    loader.registerResourceManager("SHADER", &shaderManager);
    loader.registerResourceManager("TEXTURE", &textureManager);
    loader.registerResourceManager("SPRITE", &spriteManager);
    loader.init();

    const u32 renderMemSize = MEGABYTES(8);
    BitEngine::MemoryArena renderArena;
    renderArena.init((u8*)malloc(renderMemSize), renderMemSize);
    memset(renderArena.base, 0, renderArena.size);
    RenderQueue renderQueue(renderArena);

    GLRenderer renderer;

    gameMemory.loader = &loader;
    gameMemory.videoSystem = &video;
    gameMemory.window = main_window;
    gameMemory.engineConfig = &engineConfig;
    gameMemory.taskManager = &taskManager;
    gameMemory.commandSystem = &commandSystem;
    gameMemory.imGuiRender = &imgui;
    gameMemory.logger = GameLog();
    gameMemory.renderQueue = &renderQueue;

    auto imguiMenu = [&](const BitEngine::ImGuiRenderEvent& event) {
        resourceManagerMenu("Sprite Manager", &spriteManager);
        resourceManagerMenu("Texture Manager", &textureManager);
        resourceManagerMenu("Shader Manager", &shaderManager);
    };
    imgui.subscribe(imguiMenu);

    bool rendererReady = false;
    
    {
        MyGame* game = new MyGame(&gameMemory);

        bool32 running = true;

        while (running) {
            BE_PROFILE_SCOPE("Game Loop");

            input.update();

            main_window->drawBegin();

            running = game->update();
            
            if (running) {
                BE_PROFILE_SCOPE("Game Render");
                if (!rendererReady) {
                    // TODO: Clean this up, maybe have a platform index loaded previously so we can
                    // TODO: call init right after?
                    renderer.init(&loader);
                    rendererReady = true;
                }
                renderer.render(gameMemory.renderQueue);
                renderQueue.clear();

                imgui.update();

                main_window->drawEnd();
            }
        }
        delete game;
    }
    taskManager.shutdown();
}

int main(int argc, const char* argv[])
{
    BitEngine::Profiling::BeginSession("GAME");
    BitEngine::LoggerSetup::Setup(argc, argv);

    {
        BE_PROFILE_SCOPE("GAME");

        MainMemory gameMemory = {};
        gameMemory.memorySize = MEGABYTES(512);
        gameMemory.memory = malloc(gameMemory.memorySize);
        memset(gameMemory.memory, 0, gameMemory.memorySize);

        gameExecute(gameMemory);

        free(gameMemory.memory);
    }

    BitEngine::Profiling::EndSession();
    return 0;
}
