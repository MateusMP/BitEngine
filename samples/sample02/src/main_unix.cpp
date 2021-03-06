
#include <string>
#include <fstream>

#include <BitEngine/bitengine.h>
#include <BitEngine/Core/Messenger.h>
#include <BitEngine/Core/GeneralTaskManager.h>
#include <BitEngine/Common/TypeDefinition.h>
#include <BitEngine/Core/Resources/DevResourceLoader.h>

#define GL2_API
#ifdef GL2_API
#include <Platform/opengl/GL2/GL2Driver.h>
#include <Platform/opengl/GL2/GL2ShaderManager.h>
#include <Platform/opengl/GL2/GL2TextureManager.h>
#endif

#include <Platform/glfw/GLFW_VideoSystem.h>
#include <Platform/glfw/GLFW_InputSystem.h>
#include <Platform/glfw/GLFW_ImGuiSystem.h>

#include "GamePlatform/OpenGL/OpenGLRenderer.h"
#include "GamePlatform/AssimpMeshManager.h"
#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"
#include "imgui.h"

#include <BitEngine/Global/globals.cpp>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dlfcn.h>

inline __time_t WindowsFileLastWriteTime(const char* FileName)
{
    struct stat attrib;
    stat(FileName, &attrib);
    // char date[10];
    // strftime(date, 10, "%d-%m-%y", gmtime(&(attrib.st_ctime)));
    // printf("The file %s was last modified at %s\n", FileName, date);
    // date[0] = 0;

    return attrib.st_ctime;
}

typedef bool (*GAME_CALL)(MainMemory* memory);

struct Game {
    void* dll;
    GAME_CALL setup;
    GAME_CALL update;
    GAME_CALL shutdown;
    __time_t time;
    bool valid;
    bool change;
    std::string fname;
};

BitEngine::Logger* GameLog()
{
    static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
    return &log;
}

#include "Game/Common/CommonMain.h"

Game loadGameCode(const char* path, Game& current)
{
    Game game = {};

    std::string actualLoad = std::string(path) + ".tmp.so";
    actualLoad[actualLoad.size() - 5] = current.change ? '0' : '1';

    while (true) {
        printf("Creating file copy %s -> %s\n", path, actualLoad.c_str());
        fflush(stdout);
        std::ifstream src(path, std::ios::binary);
        std::ofstream dst(actualLoad, std::ios::binary);
        if (!src.is_open() && !dst.is_open())
            continue;
        dst << src.rdbuf();
        printf("Creating file copy DONE\n");
        fflush(stdout);
        break;
    }

    void* gameDLL = dlopen(actualLoad.c_str(), RTLD_LAZY);
    if (gameDLL) {
        game.dll = gameDLL;
        game.setup = (GAME_CALL)dlsym(gameDLL, "GAME_SETUP");
        game.update = (GAME_CALL)dlsym(gameDLL, "GAME_UPDATE");
        game.shutdown = (GAME_CALL)dlsym(gameDLL, "GAME_SHUTDOWN");
        game.valid = game.update != nullptr && game.setup != nullptr && game.shutdown != nullptr;
        game.time = WindowsFileLastWriteTime(path);
        game.change = !current.change;
        game.fname = actualLoad;
    }
    else {
        char* e = dlerror();
        printf("ERROR: %s\n", e);
    }
    return game;
}

void unloadGameCode(Game& game)
{
    dlclose(game.dll);
    game.valid = false;
    if (0 != remove(game.fname.c_str())) {
        printf("Failed to delete old dll.\n");
    }
}

void game()
{
    printf("Starting...\n");
    fflush(stdout);
    // Basic infrastructure
    BitEngine::EngineConfigurationFileLoader configurations("config.ini");
    BitEngine::GeneralTaskManager taskManager;

    BitEngine::EngineConfiguration engineConfig;
    printf("Loading configs...");
    fflush(stdout);
    configurations.loadConfigurations(engineConfig);
    printf("configs ready\n");
    fflush(stdout);

    BitEngine::GLFW_VideoSystem video;
    BitEngine::GLFW_ImGuiSystem imgui;
    video.init();

    BitEngine::Window* main_window;

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

    printf("Window ready...\n");
    fflush(stdout);

    BitEngine::GLFW_InputSystem input;
    input.registerWindow(main_window);

    // Game Specific stuff
    BitEngine::CommandSystem commandSystem(main_window);

    BitEngine::GL2ShaderManager shaderManager(&taskManager);
    BitEngine::GL2TextureManager textureManager(&taskManager);
    BitEngine::SpriteManager spriteManager;
    AssimpMeshManager modelManager(&taskManager);

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
    loader.registerResourceManager("MODEL3D", &modelManager);
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
    gameMemory.profiler = &BitEngine::Profiling::Get();
    gameMemory.imGuiContext = imgui.getContext();
    gameMemory.renderQueue = &renderQueue;

    auto imguiMenu = [&](const BitEngine::ImGuiRenderEvent& event) {
        resourceManagerMenu("Sprite Manager", &spriteManager);
        resourceManagerMenu("Texture Manager", &textureManager);
        resourceManagerMenu("Shader Manager", &shaderManager);
    };
    imgui.subscribe(imguiMenu);

    GLRenderer renderer;

    printf("Loading dll...\n");
    fflush(stdout);
    const char* gameDll = "libSample02DLL.so";
    Game game = loadGameCode(gameDll, game);
    if (!game.valid) {
        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Failed to load dll";
        abort();
    }
    game.setup(&gameMemory);

    int nticks = 200;

    bool rendererReady = false;

    printf("Entering main loop...\n");
    fflush(stdout);
    bool32 running = true;
    while (running) {
        BE_PROFILE_SCOPE("Game Loop");
        {
            BE_PROFILE_SCOPE("DLL Reload");
            __time_t NewDLLWriteTime = WindowsFileLastWriteTime(gameDll);
            if (nticks <= 0 && (NewDLLWriteTime - game.time) != 0) {
                game.shutdown(&gameMemory);
                unloadGameCode(game);
                sleep(1);
                Game newGame = loadGameCode(gameDll, game);
                if (newGame.valid) {
                    printf("Game code reloaded!\n");
                    game = newGame;
                    newGame.setup(&gameMemory);
                }
                else {
                    printf("Failed to reload game code!");
                }
                fflush(stdout);
                nticks = 200;
            }
            nticks--;
        }

        // Game loop logic
        input.update();
        printf("Input done!\n");
        fflush(stdout);

        main_window->drawBegin();

        printf("Draw begin!\n");
        fflush(stdout);
        running = game.update(&gameMemory);
        printf("Game update!\n");
        fflush(stdout);

        if (running) {
            printf("Loop!\n");
            fflush(stdout);
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
}

int main(int argc, const char* argv[])
{
    printf("Setup");
    fflush(stdout);
    BitEngine::Profiling::ChromeProfiler profiler;
    BitEngine::Profiling::SetInstance(&profiler);
    BitEngine::Profiling::BeginSession("GAME");
    BitEngine::LoggerSetup::Setup(argc, argv);
    printf("Setup ok");
    fflush(stdout);

    game();

    BitEngine::Profiling::EndSession();
    return 0;
}
