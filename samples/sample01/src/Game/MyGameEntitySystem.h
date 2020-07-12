#pragma once

#include <BitEngine/Core/Graphics/Sprite2D.h>
#include <BitEngine/Game/ECS/Camera2DProcessor.h>
#include <BitEngine/Game/ECS/Camera3DProcessor.h>
#include <BitEngine/Game/ECS/GameLogicProcessor.h>
#include <BitEngine/Game/ECS/RenderableMeshComponent.h>
#include <BitEngine/Game/ECS/EntitySystem.h>


#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"

class MyGameEntitySystem;



struct PlayerControlComponent : public BitEngine::Component<PlayerControlComponent>
{
    float movH, movV;

    BitEngine::ComponentRef<BitEngine::Transform2DComponent> transform2d;
};

struct GameState {

    bool32 initialized;
    bool32 running;

    BitEngine::ResourceLoader *resources;

    BitEngine::MemoryArena mainArena;
    BitEngine::MemoryArena resourceArena;
    BitEngine::MemoryArena permanentArena;
    BitEngine::MemoryArena entityArena;

    MyGameEntitySystem* entitySystem;

    UserGUI* m_userGUI;
    GameWorld* m_world; //!< Current active world

    BitEngine::ComponentRef<PlayerControlComponent> playerControl;

    PlayerCamera* m_camera3d;
};

struct UserRequestQuitGame {
    GameQuitType quitType;
};


struct Mesh3D : BitEngine::Component<Mesh3D> {

};


class BE_API Sprite2DProcessor
{
public:
    constexpr static u32 DEFAULT_SPRITE = 0;
    constexpr static u32 TRANSPARENT_SPRITE = 1;
    constexpr static u32 EFFECT_SPRITE = 2;

    static void Process(BitEngine::EntitySystem* es, BitEngine::ComponentRef<BitEngine::Camera2DComponent>& camera, RenderQueue* queue) {
        BE_PROFILE_FUNCTION();

        Render2DBatchCommand* batch = queue->initRenderBatch2D();

        const glm::vec4 viewScreen = camera->getWorldViewArea();
        batch->view = camera->getMatrix();

        // Build batch
        // TODO: Make loop append entries to render queue
        // Individual entries should be prepared for rendering
        // by the rendering implementation
        es->forEach<BitEngine::SceneTransform2DComponent, BitEngine::Sprite2DComponent>(
            [=](BitEngine::ComponentRef<BitEngine::SceneTransform2DComponent>&& transform, BitEngine::ComponentRef<BitEngine::Sprite2DComponent>&& sprite)
        {
            if (insideScreen(viewScreen, transform->getGlobal(), 64))
            {
                queue->pushSprite2D(batch, sprite->sprite.get(), sprite->alpha, sprite->layer, sprite->material, transform->getGlobal());
            }
        });


    }

private:

    static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, float radius)
    {
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

    BitEngine::ResourceLoader* m_resourceLoader;
};

class Mesh3DProcessor
{
public:
    Mesh3DProcessor(BitEngine::Transform3DProcessor *t3dp_)
        : t3dp(t3dp_)
    {
    }

    void processEntities(BitEngine::EntitySystem* es, RenderQueue* queue, const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& activeCamera)
    {
        using namespace BitEngine;

        // Find out the 2D camera

        // printf("3d To render: %d\n", nObjs);
        Render3DBatchCommand* batch = queue->initRenderBatch3D();
        batch->projection = activeCamera->getProjection();
        batch->view = activeCamera->getView();

        batch->light.mode = LightMode::SUNLIGHT;
        batch->light.position = { cos(f)*300, -50.0f, 0 };
        f += 0.03f;
        batch->light.direction = { cos(f), 0.5f, -0.2f };
        batch->light.color = { 1.f, 1.f, 1.f };

        es->forEach<RenderableMeshComponent, Transform3DComponent>(
            [&](ComponentRef<RenderableMeshComponent>&& renderable, ComponentRef<Transform3DComponent>&& transform)
        {
            RR<Model> model = renderable->getModel();
            if (model->getMeshCount() == 0) {
                return;
            }
            if (renderable->getMesh()) {
                Model3D *m = queue->pushModel3D(batch);
                m->mesh = renderable->getMesh().get();
                m->material = renderable->getMaterial();
                m->transform = t3dp->getGlobalTransformFor(transform.getComponentID());
            } else {
                for (u32 i = 0; i < model->getMeshCount(); ++i) {
                    Model3D *m = queue->pushModel3D(batch);
                    m->mesh = model->getMesh(i);
                    if (renderable->getMaterial() == nullptr) {
                        m->material = m->mesh->getMaterial();
                    } else {
                        m->material = renderable->getMaterial();
                    }
                    m->transform = t3dp->getGlobalTransformFor(transform.getComponentID());
                }
            }
        });
    }

private:
    BitEngine::Transform3DProcessor *t3dp;
    float f;
};


void PlayerControlSystem(BitEngine::EntitySystem* es)
{
    BE_PROFILE_FUNCTION();
    es->forAll<PlayerControlComponent>([](BitEngine::ComponentHandle id, PlayerControlComponent& comp) {
        float vel = 2.0f;

        // camT2D->setPosition(x, y);
        BitEngine::Vec2 pos = comp.transform2d->getLocalPosition();
        pos.x += comp.movH * vel;
        pos.y += comp.movV * vel;

        comp.transform2d->setLocalPosition(pos);
        comp.transform2d->setLocalRotation(comp.transform2d->getLocalRotation() + 0.03f);
    });
}

class SpinnerComponent : public BitEngine::Component<SpinnerComponent>
{
public:
    SpinnerComponent() {

    }
    SpinnerComponent(float _speed) : speed(_speed) {

    }

    float speed;
};

void SpinnerSystem(BitEngine::EntitySystem *es)
{
    using namespace BitEngine;
    es->forEach<Transform2DComponent, SpinnerComponent>(
        [=](ComponentRef<Transform2DComponent> transform, const ComponentRef<SpinnerComponent> spinner)
    {
        transform->setLocalRotation(transform->getLocalRotation() + spinner->speed*1);
    });
}

class MyComponentsRegistry : public BitEngine::EntitySystem {
public:
    MyComponentsRegistry() {
        registerComponents();
    }
    void registerComponents() {
        using namespace BitEngine;
        registerComponent<PlayerControlComponent>();
        registerComponent<GameLogicComponent>();
        registerComponent<SpinnerComponent>();
        registerComponent<SceneTransform2DComponent>();
        registerComponent<RenderableMeshComponent>();
        registerComponent<Sprite2DComponent>();
        registerComponent<Camera2DComponent>();
        registerComponent<Camera3DComponent>();
        registerComponent<Transform2DComponent>();
        registerComponent<Transform3DComponent>();
    }
};

class MyGameEntitySystem : public MyComponentsRegistry
{
public:
    MyGameEntitySystem(BitEngine::ResourceLoader* loader, BitEngine::MemoryArena* entityMemory)
        : MyComponentsRegistry(),
        t2p(this), t3p(this),
        cam2Dprocessor(this, &t2p), cam3Dprocessor(this, &t3p),
        mesh3dSys(&t3p)
    {
        using namespace BitEngine;
    }

    // Processors
    BitEngine::Transform2DProcessor t2p;
    BitEngine::Transform3DProcessor t3p;
    BitEngine::Camera2DProcessor cam2Dprocessor;
    BitEngine::Camera3DProcessor cam3Dprocessor;
    Mesh3DProcessor mesh3dSys;
};


BitEngine::EntityHandle CreatePlayerTemplate(BitEngine::ResourceLoader* loader, MyGameEntitySystem* es, BitEngine::CommandSystem* cmdSys)
{
    using namespace BitEngine;

    RR<Sprite> playerSPR = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox");
    RR<Sprite> playerOrbitSPR = loader->getResource<BitEngine::Sprite>("data/sprites/spr_skybox_orbit");

    EntityHandle ent_player;
    ComponentRef<Transform2DComponent> playerT2D;
    ComponentRef<SceneTransform2DComponent> playerST2D;
    ComponentRef<Sprite2DComponent> playerSpr2D;
    ComponentRef<GameLogicComponent> gamelogic;
    ComponentRef<PlayerControlComponent> playerControl;

    ent_player = es->createEntity();
    LOG(GameLog(), BE_LOG_VERBOSE) << "ent_player: " << ent_player;

    // 2D
    BE_ADD_COMPONENT_ERROR(playerT2D = es->addComponent<Transform2DComponent>(ent_player));
    BE_ADD_COMPONENT_ERROR(playerST2D = es->addComponent<SceneTransform2DComponent>(ent_player));
    BE_ADD_COMPONENT_ERROR(playerSpr2D = es->addComponent<Sprite2DComponent>(ent_player));
    BE_ADD_COMPONENT_ERROR(playerControl = es->addComponent<PlayerControlComponent>(ent_player));

    playerT2D->setLocalPosition(0, 0);
    playerSpr2D->layer = 5;
    playerSpr2D->sprite = playerSPR;

    playerControl->movH = 0;
    playerControl->movV = 0;
    playerControl->transform2d = playerT2D;

    // 2D orbit
    ComponentRef<Transform2DComponent> pcT;
    ComponentRef<Sprite2DComponent> pcS;
    ComponentRef<SceneTransform2DComponent> pcST;
    EntityHandle playerConnected = es->createEntity();
    BE_ADD_COMPONENT_ERROR(pcT = es->addComponent<Transform2DComponent>(playerConnected));
    BE_ADD_COMPONENT_ERROR(pcST = es->addComponent<SceneTransform2DComponent>(playerConnected));
    BE_ADD_COMPONENT_ERROR(pcS = es->addComponent<Sprite2DComponent>(playerConnected));
    es->t2p.setParentOf(pcT, playerT2D);
    pcT->setLocalPosition(128, 128);
    pcT->setLocalRotation(45 * 3.1415f / 180.0f);
    pcS->layer = 6;
    pcS->sprite = playerOrbitSPR;

    return ent_player;
}