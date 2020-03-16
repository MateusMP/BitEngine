#pragma once

#include <imgui.h>

#include <BitEngine/Core/VideoSystem.h>
#include <BitEngine/Core/Graphics/Sprite2D.h>
#include <BitEngine/Game/ECS/EntitySystem.h>

#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"
#include "Overworld.h"

class UserGUI
{
public:
    UserGUI(MyGameEntitySystem* es)
    {
        gui = es->createEntity();

        camera = es->addComponent<BitEngine::Camera2DComponent>(gui);

        camera->setView(1280, 720);
        camera->setLookAt(glm::vec3(1280 / 2, 720 / 2, 0));
        camera->setZoom(1.0f);
    }

    BitEngine::ComponentRef<BitEngine::Camera2DComponent>& getCamera() {
        return camera;
    }

private:
    BitEngine::EntityHandle gui;
    BitEngine::ComponentRef<BitEngine::Camera2DComponent> camera;
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

private:
    std::function<void()> f;
};


void resourceLoaderMenu(const char* name, BitEngine::ResourceLoader* loader) {
    constexpr float TO_MB = 1.0 / (1024 * 1024);
    if (ImGui::TreeNode(name)) {
        ImGui::TextColored(ImVec4(1, 1, 0, 1), "Resources pending load: %lu", loader->getPendingToLoad().size());
        for (auto p : loader->getPendingToLoad()) {
            if (ImGui::TreeNode(p.first->getNameId().c_str())) {
                ImGui::TextColored(ImVec4(1, 1, 0, 1), "Task waiting for %lu dependencies", p.second->getDependencies().size());
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

class MyGame :
    BitEngine::Messenger<BitEngine::CommandSystem::MsgCommandInput>::ScopedSubscription,
    BitEngine::Messenger<BitEngine::WindowClosedEvent>::ScopedSubscription,
    BitEngine::Messenger<BitEngine::ImGuiRenderEvent>::ScopedSubscription
{
public:
    MyGame(MainMemory* gameMemory)
        : BitEngine::Messenger<BitEngine::CommandSystem::MsgCommandInput>::ScopedSubscription(gameMemory->commandSystem->commandSignal, &MyGame::onMessage, this),
        BitEngine::Messenger<BitEngine::WindowClosedEvent>::ScopedSubscription(gameMemory->window->windowClosedSignal, &MyGame::onMessage, this),
        BitEngine::Messenger<BitEngine::ImGuiRenderEvent>::ScopedSubscription(*gameMemory->imGuiRender, &MyGame::onMessage, this),
        mainMemory(gameMemory)
    {
        gameState = (GameState*)gameMemory->memory;

        if (gameState->initialized) {
            gameState->entitySystem->registerComponents();
        }
    }

    ~MyGame() {

    }

    void onMessage(const BitEngine::ImGuiRenderEvent& ev)
    {
        static bool active = true;
        ImGui::Begin("Overview", &active, ImGuiWindowFlags_MenuBar);

        if (ImGui::CollapsingHeader("Tasks"))
        {
            auto taskManager = mainMemory->taskManager;
            // Display contents in a scrolling region
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "Tasks: %lu", taskManager->getTasks().size());
            ImGui::BeginChild("Scrolling");
            for (const BitEngine::TaskPtr& ptr : taskManager->getTasks()) {
                ImGui::Text("Task, deps: %04lu", ptr->getDependencies().size());
            }
            ImGui::EndChild();
        }

        if (ImGui::CollapsingHeader("Resources"))
        {
            resourceLoaderMenu("Loader", mainMemory->loader);
        }

        ImGui::End();
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


    bool init()
    {
        BE_PROFILE_FUNCTION();
        using namespace BitEngine;

        // Create memory arenas
        gameState->mainArena.init((u8*)mainMemory->memory + sizeof(GameState), mainMemory->memorySize - sizeof(GameState));
        gameState->permanentArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(8)), MEGABYTES(8));
        gameState->entityArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(64)), MEGABYTES(64));
        gameState->resourceArena.init((u8*)gameState->mainArena.alloc(MEGABYTES(256)), MEGABYTES(256));
        gameState->initialized = true;

        setupCommands(mainMemory->commandSystem);

        MemoryArena& permanentArena = gameState->permanentArena;

        auto loader = mainMemory->loader;
        loader->loadIndex("../data/main.idx");

        mainMemory->taskManager->addTask(std::make_shared<UpdateTask>([loader] {loader->update(); }));

        // Init game state stuff
        gameState->entitySystem = permanentArena.push<MyGameEntitySystem>(loader, &gameState->entityArena);
        gameState->entitySystem->init();

        gameState->m_userGUI = permanentArena.push<UserGUI>(gameState->entitySystem);
        gameState->m_world = permanentArena.push<GameWorld>(mainMemory, gameState->entitySystem);

        gameState->m_player = permanentArena.push<Player>("nick_here", 0);
        gameState->m_world->addPlayer(gameState->m_player);
        gameState->m_camera3d = permanentArena.push<PlayerCamera>(gameState->entitySystem);
        gameState->m_world->setActiveCamera(gameState->m_camera3d->getCamera());
        gameState->m_camera3d->setLookAt({ 0,0,0 });
        gameState->m_camera3d->getTransform()->setPosition({ 50,0,300 });

        // Tests
        const RR<Texture> texture = loader->getResource<BitEngine::Texture>("texture.png");
        const RR<Texture> texture2 = loader->getResource<BitEngine::Texture>("sun.png");
        if (!texture.isValid() || !texture2.isValid()) {
            return false;
        }

        LOG(GameLog(), BE_LOG_VERBOSE) << "Texture loaded: " << texture->getTextureID();

        RR<Model> model = loader->getResource<Model>("rocks_model");
        for (int i = 0; i < 4; ++i) {
            auto entity = gameState->entitySystem->createEntity();
            auto transform = gameState->entitySystem->addComponent<Transform3DComponent>(entity);
            gameState->entitySystem->addComponent<RenderableMeshComponent>(entity, model);
            transform->setPosition(-300 + i * 180, -20, -200);
        }

        RR<Sprite> spr1 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox");
        RR<Sprite> spr2 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_orbit");
        RR<Sprite> spr3 = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_piece");

        //BitEngine::SpriteHandle spr1 = loader->getResource<BitEngine::Sprite>("player", BitEngine::Sprite(texture, 128, 128, 0.5f, 0.5f, glm::vec4(0, 0, 1, 1)));
        //BitEngine::SpriteHandle spr2 = sprMng->createSprite("playerOrbit", BitEngine::Sprite(texture, 640, 64, 0.5f, 0.0f, glm::vec4(0, 0, 1.0f, 1.0f)));
        //BitEngine::SpriteHandle spr3 = sprMng->createSprite(BitEngine::Sprite(texture2, 256, 256, 0.5f, 0.5f, glm::vec4(0, 0, 2.0f, 2.0f), true));

        // CREATE PLAYER
        auto playerEntity = PlayerControlSystem::CreatePlayerTemplate(loader, gameState->entitySystem, mainMemory->commandSystem);
        gameState->playerControl = gameState->entitySystem->getComponentRef<PlayerControlComponent>(playerEntity);



        // Sparks
        MyGameEntitySystem* es = gameState->entitySystem;
        for (int i = 0; i < 9; ++i)
        {
            BitEngine::EntityHandle h = gameState->entitySystem->createEntity();
            BitEngine::ComponentRef<BitEngine::Transform2DComponent> transformComp;
            BitEngine::ComponentRef<BitEngine::Sprite2DComponent> spriteComp;
            BitEngine::ComponentRef<BitEngine::SceneTransform2DComponent> sceneComp;
            BitEngine::ComponentRef<BitEngine::GameLogicComponent> logicComp;
            BE_ADD_COMPONENT_ERROR(transformComp = es->addComponent<BitEngine::Transform2DComponent>(h));
            BE_ADD_COMPONENT_ERROR(spriteComp = es->addComponent<BitEngine::Sprite2DComponent>(h, 6, spr3, nullptr)); // es->spr2D.getMaterial(Sprite2DRenderer::EFFECT_SPRITE)
            BE_ADD_COMPONENT_ERROR(sceneComp = es->addComponent<BitEngine::SceneTransform2DComponent>(h));
            BE_ADD_COMPONENT_ERROR(logicComp = es->addComponent<BitEngine::GameLogicComponent>(h));
            BE_ADD_COMPONENT_ERROR(es->addComponent<SpinnerComponent>(h, (rand() % 10) / 100.0f + 0.02f));

            transformComp->setLocalPosition(i * 128 + 125, 500);
            spriteComp->alpha = 1.0;
        }

        gameState->running = true;
        return true;
    }

    bool32 update()
    {
        if (!gameState->initialized) {
            init();
            gameState->initialized = true;
        }

        gameState->entitySystem->update();

        mainMemory->taskManager->update();

        // Render

        if (gameState->running) {
            render();
        }
        else {
            gameState->entitySystem->~MyGameEntitySystem();
        }

        return gameState->running;
    }

    void onMessage(const BitEngine::WindowClosedEvent& msg) {
        gameState->running = false;
    }

    void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
    {
        if (msg.commandID == RELOAD_SHADERS) {
            LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Reloading index";
            gameState->resources->loadIndex("data/main.idx");

        }

        BitEngine::ComponentRef<PlayerControlComponent>& comp = gameState->playerControl;
        switch (msg.commandID)
        {
        case RIGHT:
            comp->movH = msg.intensity;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                comp->movH = 0;
            break;
        case LEFT:
            comp->movH = -msg.intensity;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                comp->movH = 0;
            break;
        case UP:
            comp->movV = msg.intensity;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                comp->movV = 0;
            break;
        case DOWN:
            comp->movV = -msg.intensity;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                comp->movV = 0;
            break;

        case CLICK:
            printf("CLICK!!!!\n\n");
            break;
        }
    }

    void onMessage(const BitEngine::MsgFrameStart& msg)
    {
        //LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Frame Start";
    }

    void render()
    {
        BE_PROFILE_FUNCTION();
        mainMemory->renderQueue->pushCommand(SceneBeginCommand{ 0,0, BitEngine::ColorRGBA(0.3f, 0.3f, 0.3f, 0.f) });

        gameState->entitySystem->mesh3dSys.setActiveCamera(gameState->m_world->getActiveCamera());
        gameState->entitySystem->mesh3dSys.processEntities(mainMemory->renderQueue);

        gameState->entitySystem->spr2D.processEntities(gameState->m_userGUI->getCamera(), mainMemory->renderQueue);
    }

    void onMessage(const BitEngine::WindowResizedEvent& ev)
    {
        //mainMemory->videoSystem->getDriver()->setViewPort(0, 0, ev.width, ev.height);
    }

private:
    MainMemory* mainMemory;
    GameState* gameState;
};
