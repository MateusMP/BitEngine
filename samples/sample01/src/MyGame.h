#pragma once

#include "Overworld.h"

#include "MyGameSystem.h"

#include "Platform/GLFW/GLFW_ImGuiSystem.h"
#include "imgui.h"

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


void resourceLoaderMenu(const char* name, BitEngine::ResourceLoader* loader) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Resources pending load: %d", loader->getPendingToLoad().size());
        for (auto p : loader->getPendingToLoad()) {
            if (ImGui::TreeNode(p.first->getNameId().c_str())) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Task waiting for %d dependencies", p.second->getDependencies().size());
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void resourceManagerMenu(const char* name, BitEngine::ResourceManager *resMng) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "RAM: %.2f GPU: %.2f", resMng->getCurrentRamUsage()* TO_MB, resMng->getCurrentGPUMemoryUsage()*TO_MB);
        ImGui::TreePop();
    }
}

class MyGame : public BitEngine::MessengerEndpoint
{
public:
    MyGame(MainMemory* gameMemory)
        : MessengerEndpoint(gameMemory->messenger), gameMemory(gameMemory), running(false)
    {
        subscribe<BitEngine::CommandSystem::MsgCommandInput>(&MyGame::onMessage, this);
        subscribe<RenderEvent>(&MyGame::onMessage, this);
        subscribe<BitEngine::WindowClosedEvent>(&MyGame::onMessage, this);
        subscribe<BitEngine::ImGuiRenderEvent>(&MyGame::onMessage, this);

        gameState = (GameState*)gameMemory->memory;

        // Create the main arena with all memory!
        gameState->mainArena.init((u8*)gameMemory->memory + sizeof(GameState), gameMemory->memorySize - sizeof(GameState));
        gameState->permanentArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(8)), MEGABYTES(8));
        gameState->entityArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(64)), MEGABYTES(64));
        gameState->resourceArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(256)), MEGABYTES(256));
    }


    void onMessage(const BitEngine::ImGuiRenderEvent& ev)
    {
        static bool active = true;
        ImGui::Begin("Overview", &active, ImGuiWindowFlags_MenuBar);

        if (ImGui::CollapsingHeader("Tasks"))
        {
            auto taskManager = gameMemory->taskManager;
            // Display contents in a scrolling region
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Tasks: %d", taskManager->getTasks().size());
            ImGui::BeginChild("Scrolling");
            for (const BitEngine::TaskPtr& ptr : taskManager->getTasks()) {
                ImGui::Text("Task, deps: %04d", ptr->getDependencies().size());
            }
            ImGui::EndChild();
        }

        if (ImGui::CollapsingHeader("Resources"))
        {
            resourceLoaderMenu("Loader", gameState->resources);
            resourceManagerMenu("Sprite Manager", gameMemory->spriteManager);
            resourceManagerMenu("Texture Manager", gameMemory->textureManager);
            resourceManagerMenu("Shader Manager", gameMemory->shaderManager);
        }

        ImGui::End();
    }

    ~MyGame()
    {
    }

    bool init()
    {
        using namespace BitEngine;

        MemoryArena& permanentArena = gameState->permanentArena;

        ResourceLoader* loader = permanentArena.push<DevResourceLoader>(gameState->resourceArena, gameMemory->messenger, gameMemory->taskManager);
        gameState->resources = loader;
        loader->registerResourceManager("SHADER", gameMemory->shaderManager);
        loader->registerResourceManager("TEXTURE", gameMemory->textureManager);
        loader->registerResourceManager("SPRITE", gameMemory->spriteManager);
        loader->init();
        loader->loadIndex("data/main.idx");

        gameMemory->taskManager->addTask(std::make_shared<UpdateTask>([loader] {loader->update(); }));

        // Init game state stuff
        gameState->entitySystem = permanentArena.push<MyGameEntitySystem>(loader, gameMemory->messenger, &gameState->entityArena);
        gameState->entitySystem->Init();

        gameState->m_userGUI = permanentArena.push<UserGUI>(gameState->entitySystem);
        gameState->m_world = permanentArena.push<GameWorld>(gameState->entitySystem);

        gameState->selfPlayer = permanentArena.push<Player>("nick_here", 0);
        gameState->m_world->addPlayer(gameState->selfPlayer);

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
            ADD_COMPONENT_ERROR(es->AddComponent<SpinnerComponent>(h, (rand() % 10) / 100.0f + 0.02f));

            transformComp->setLocalPosition(i * 128 + 125, 500);
            spriteComp->alpha = 1.0;
        }

        gameState->m_world->start();

        running = true;
        return true;
    }

    void shutdown() {
        gameState->entitySystem->shutdown();
        gameState->resources->shutdown();
    }

    bool32 update()
    {
        if (!gameState->initialized) {
            init();
            gameState->initialized = true;
        }

        gameState->entitySystem->update();

        gameMemory->taskManager->update();

        // Render

        if (running) {
            getMessenger()->emit<RenderEvent>(RenderEvent{ gameState });
        }
        else {
            shutdown();
        }

        return running;
    }

    void onMessage(const BitEngine::WindowClosedEvent& msg) {
        running = false;
    }

    void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
    {
        LOG(GameLog(), BE_LOG_VERBOSE) << "Command: " << msg.commandID;
        if (msg.commandID == RELOAD_SHADERS) {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Reloading index";
            gameState->resources->loadIndex("data/main.idx");

        }
    }

    void onMessage(const BitEngine::MsgFrameStart& msg)
    {
        //LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Frame Start";
    }

    void onMessage(const RenderEvent& ev)
    {
        BitEngine::VideoDriver* driver = gameMemory->videoSystem->getDriver();
        driver->clearBufferColor(nullptr, BitEngine::ColorRGBA(0.5f, 0.2f, 0.3f, 0.f));
        driver->clearBuffer(nullptr, BitEngine::BufferClearBitMask::COLOR_DEPTH);

        //ev.state->entitySystem->sh3D.setActiveCamera(ev.state->m_world->getActiveCamera());
        //ev.state->entitySystem->sh3D.Render();

        ev.state->entitySystem->spr2D.setActiveCamera(ev.state->m_userGUI->getCamera());
        ev.state->entitySystem->spr2D.Render(driver);
    }

    void onMessage(const BitEngine::WindowResizedEvent& ev)
    {
        gameMemory->videoSystem->getDriver()->setViewPort(0, 0, ev.width, ev.height);
    }

private:
    MainMemory* gameMemory;
    GameState* gameState;
    bool32 running;
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