#define CR_HOST // required in the host only and before including cr.h
#include <cr/cr.h>

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

BitEngine::Logger* GameLog()
{
    static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
    return &log;
}

#include "Game/Common/CommonMain.h"

void game()
{
    // Basic infrastructure
    BitEngine::EngineConfigurationFileLoader configurations("config.ini");
    BitEngine::GeneralTaskManager taskManager;

    BitEngine::EngineConfiguration engineConfig;
    configurations.loadConfigurations(engineConfig);

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
    gameMemory.memory = ::operator new(gameMemory.memorySize);
    memset(gameMemory.memory, 0, gameMemory.memorySize);

    // Setup Resource loader

    const u32 resMemSize = MEGABYTES(64);
    BitEngine::MemoryArena resourceArena;
    resourceArena.init((u8*)::operator new(resMemSize), resMemSize);
    memset(resourceArena.base, 0, resMemSize);

    BitEngine::DevResourceLoader loader(&taskManager, resourceArena);
    loader.registerResourceManager("SHADER", &shaderManager);
    loader.registerResourceManager("TEXTURE", &textureManager);
    loader.registerResourceManager("SPRITE", &spriteManager);
    loader.registerResourceManager("MODEL3D", &modelManager);
    loader.init();

    const u32 renderMemSize = MEGABYTES(8);
    BitEngine::MemoryArena renderArena;
    renderArena.init((u8*)::operator new(renderMemSize), renderMemSize);
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
    gameMemory.imGuiRender = &imgui.events;
    gameMemory.logger = GameLog();
    gameMemory.profiler = &BitEngine::Profiling::Get();
    gameMemory.imGuiContext = imgui.getContext();
    gameMemory.renderQueue = &renderQueue;

    auto imguiMenu = [&](const BitEngine::ImGuiRenderEvent& event) {
        resourceManagerMenu("Sprite Manager", &spriteManager);
        resourceManagerMenu("Texture Manager", &textureManager);
        resourceManagerMenu("Shader Manager", &shaderManager);
    };
    imgui.events.subscribe(imguiMenu);

    auto imguiRenderQueue = [&](const BitEngine::ImGuiRenderEvent& event) {

        ImGui::Begin("RenderQueue");

        auto queue = gameMemory.renderQueue;
        ptrsize cmdCount = queue->getCommandsCount();

        for (int cmd = 0; cmd < cmdCount; ++cmd) {
            RenderCommand& command = queue->getCommands()[cmd];
            if (ImGui::TreeNodeEx(&command, ImGuiTreeNodeFlags_OpenOnArrow, "%s", GetCommandName(command.command))) {
                switch (command.command) {
                case Command::SCENE_BEGIN: {
                    SceneBeginCommand* sceneBegin = command.dataAs<SceneBeginCommand>();
                    ImGui::ColorEdit4("Clear Color", (float*)&sceneBegin->color, ImGuiColorEditFlags_NoOptions);
                    ImGui::Text("Width %u", sceneBegin->renderWidth);
                    ImGui::Text("Height %u", sceneBegin->renderHeight);
                } break;
                case Command::SPRITE_BATCH_2D:
                    ImGui::Text("Sprites %d", command.dataAs<Render2DBatchCommand>()->batch.count);
                    break;
                case Command::SCENE_3D_BATCH:
                    ImGui::Text("Models %d", command.dataAs<Render3DBatchCommand>()->batch.count);
                    break;
                }

                ImGui::TreePop();
            }
        }
        ImGui::ShowDemoWindow();

        ImGui::End();

    };
    imgui.events.subscribe(imguiRenderQueue);

    GLRenderer renderer;

    // Load game code
    // the host application should initalize a plugin with a context, a plugin
    cr_plugin ctx;
    ctx.userdata = &gameMemory;

    // the full path to the live-reloadable application
    cr_plugin_open(ctx, "Sample02DLL.dll");

    int nticks = 200;

    bool rendererReady = false;

    bool32 running = true;

    while (running) {
        BE_PROFILE_SCOPE("Game Loop");

        // Game loop logic
        input.update();

        main_window->drawBegin();

        running = cr_plugin_update(ctx);

        if (running) {
            if (!rendererReady) {
                // TODO: Clean this up, maybe have a platform index loaded previously so we can
                // TODO: call init right after?
                renderer.init(&loader);
                rendererReady = true;
            }

            renderer.render(gameMemory.renderQueue);

            imgui.update();

            renderQueue.clear();

            main_window->drawEnd();
        }
    }

    // at the end do not forget to cleanup the plugin context
    cr_plugin_close(ctx);

    taskManager.shutdown();

    ::operator delete((void*)renderArena.base, renderMemSize);
    ::operator delete((void*)resourceArena.base, resMemSize);
    ::operator delete(gameMemory.memory, gameMemory.memorySize);
}

int main(int argc, const char* argv[])
{
    BitEngine::Profiling::ChromeProfiler profiler;
    BitEngine::Profiling::SetInstance(&profiler);
    BitEngine::Profiling::BeginSession("GAME");
    BitEngine::LoggerSetup::Setup(argc, argv);

    game();

    BitEngine::Profiling::EndSession();
    return 0;
}
