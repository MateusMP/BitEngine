
#include <string>
#include <fstream>
#include <Windows.h>

#include <Bitengine/bitengine.h>
#include <Bitengine/Core/Messenger.h>
#include <Bitengine/Core/GeneralTaskManager.h>
#include <BitEngine/Common/TypeDefinition.h>
#include <BitEngine/Core/Resources/DevResourceLoader.h>

#define GL2_API
#ifdef GL2_API
    #include <Platform/opengl/GL2/GL2Driver.h>
    #include <Platform/opengl/GL2/GL2ShaderManager.h>
    #include <Platform/opengl/GL2/GL2TextureManager.h>
#endif

#include <BitEngine/Core/Graphics/Sprite2DRenderer.h>
#include <Platform/GLFW/GLFW_VideoSystem.h>
#include <Platform/GLFW/GLFW_InputSystem.h>
#include <Platform/GLFW/GLFW_ImGuiSystem.h>


#include "Common/MainMemory.h"
#include "Common/GameGlobal.h"
#include "imgui.h"



inline FILETIME WindowsFileLastWriteTime(const char *FileName) {
    FILETIME Result = {};
    WIN32_FILE_ATTRIBUTE_DATA Data;

    if (GetFileAttributesExA(FileName, GetFileExInfoStandard, &Data)) {
        Result = Data.ftLastWriteTime;
    }

    return Result;
}


typedef bool(*GAME_CALL)(MainMemory* memory);

struct Game {
    HINSTANCE dll;
    GAME_CALL setup;
    GAME_CALL update;
    GAME_CALL shutdown;
    FILETIME time;
    bool valid;
    bool change;
};



BitEngine::Logger* GameLog()
{
    static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
    return &log;
}

#include "Common/CommonMain.h"

Game loadGameCode(const char* path, Game &current) {
    Game game = {};

    std::string actualLoad = std::string(path) + ".tmp.dll";
    if (current.valid) {
        actualLoad[actualLoad.size() - 1] = current.change ? '0' : '1';
    }

    while (true)
    {
        std::ifstream  src(path, std::ios::binary);
        std::ofstream  dst(actualLoad, std::ios::binary);
        if (!src.is_open() && !dst.is_open()) continue;
        dst << src.rdbuf();
        break;
    }

    HINSTANCE gameDLL = LoadLibraryA(actualLoad.c_str());
    if (gameDLL) {
        game.dll = gameDLL;
        game.setup = (GAME_CALL)GetProcAddress(gameDLL, "GAME_SETUP");
        game.update = (GAME_CALL)GetProcAddress(gameDLL, "GAME_UPDATE");
        game.shutdown = (GAME_CALL)GetProcAddress(gameDLL, "GAME_SHUTDOWN");
        game.valid = game.update != nullptr && game.setup != nullptr && game.shutdown != nullptr;
        game.time = WindowsFileLastWriteTime(path);
        game.change = !current.change;
    }
    return game;
}

void unloadGameCode(Game& game) {
    FreeLibrary(game.dll);
    game.valid = false;
}


int main(int argc, const char* argv[])
{
    BitEngine::LoggerSetup::Setup(argc, argv);
    LOG_FUNCTION_TIME(GameLog());

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

    // Setup game memory
    MainMemory gameMemory = {};
    gameMemory.memorySize = MEGABYTES(512);
    gameMemory.memory = malloc(gameMemory.memorySize);
    memset(gameMemory.memory, 0, gameMemory.memorySize);

    // Setup Resource loader
    
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

    gameMemory.loader = &loader;
    gameMemory.shaderManager = &shaderManager;
    gameMemory.textureManager = &textureManager;
    gameMemory.spriteManager = &spriteManager;
    gameMemory.videoSystem = &video;
    gameMemory.window = main_window;
    gameMemory.engineConfig = &engineConfig;
    gameMemory.taskManager = &taskManager;
    gameMemory.commandSystem = &commandSystem;
    gameMemory.imGuiRender = &imgui;
    gameMemory.logger = GameLog();
    gameMemory.imGuiContext = imgui.getContext();
    gameMemory.renderQueue = &renderQueue;

    auto imguiMenu = [&](const BitEngine::ImGuiRenderEvent& event) {    
        resourceManagerMenu("Sprite Manager", &spriteManager);
        resourceManagerMenu("Texture Manager",&textureManager);
        resourceManagerMenu("Shader Manager", &shaderManager);
    };
    imgui.subscribe(imguiMenu);

    GLRenderer renderer;

    // Load game code
    const char* gameDll = "Sample02DLL.dll";
    Game game = loadGameCode(gameDll, game);
    if (!game.valid) {
        abort();
    }
    game.setup(&gameMemory);

    int nticks = 200;

    bool rendererReady = false;

    bool32 running = true;
    while (running) {

        FILETIME NewDLLWriteTime = WindowsFileLastWriteTime(gameDll);
        if (nticks <= 0 && CompareFileTime(&NewDLLWriteTime, &game.time) != 0) 
        {
            game.shutdown(&gameMemory);
            unloadGameCode(game);
            Sleep(10);
            Game newGame = loadGameCode(gameDll, game);
            if (newGame.valid) {
                printf("Game code reloaded!");
                game = newGame;
                newGame.setup(&gameMemory);
            }
            else {
                printf("Failed to reload game code!");
            }
            nticks = 200;
        }
        nticks--;

        // Game loop logic
        input.update();

        main_window->drawBegin();

        running = game.update(&gameMemory);

        if (running) {
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
    game.shutdown(&gameMemory);
    taskManager.shutdown();

    unloadGameCode(game);
    
    free(renderArena.base);
    free(gameMemory.memory);

    return 0;
}
