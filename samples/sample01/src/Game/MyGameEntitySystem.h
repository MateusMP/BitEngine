#pragma once

#include <BitEngine/Core/Graphics/Sprite2D.h>
#include <BitEngine/Core/Graphics/Sprite2DRenderer.h>
#include <BitEngine/Core/ECS/Camera2DProcessor.h>
#include <BitEngine/Core/ECS/Camera3DProcessor.h>
#include <BitEngine/Core/ECS/GameLogicProcessor.h>
#include <BitEngine/Core/ECS/RenderableMeshComponent.h>


#include "Game/Common/MainMemory.h"
#include "Game/Common/GameGlobal.h"

class MyGameEntitySystem;


struct Mesh3D : BitEngine::Component<Mesh3D> {

};



class Mesh3DProcessor : public BitEngine::ComponentProcessor
{
public:
    Mesh3DProcessor(BitEngine::EntitySystem* es, BitEngine::Transform3DProcessor *t3dp_)
        : ComponentProcessor(es), t3dp(t3dp_)
    {
    }

    void setActiveCamera(const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& camera)
    {
        activeCamera = camera;
    }

    void processEntities(RenderQueue* queue)
    {
        using namespace BitEngine;

        // Find out the 2D camera

        // printf("3d To render: %d\n", nObjs);
        Render3DBatchCommand* batch = queue->initRenderBatch3D();
        batch->projection = activeCamera->getProjection();
        batch->view = activeCamera->getView();

        getES()->forEach<RenderableMeshComponent, Transform3DComponent>(
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
                m->transform = t3dp->getGlobalTransformFor(getComponentHandle(transform));
            } else {
                for (int i = 0; i < model->getMeshCount(); ++i) {
                    Model3D *m = queue->pushModel3D(batch);
                    m->mesh = model->getMesh(i);
                    if (renderable->getMaterial() == nullptr) {
                        m->material = m->mesh->getMaterial();
                    } else {
                        m->material = renderable->getMaterial();
                    }
                    m->transform = t3dp->getGlobalTransformFor(getComponentHandle(transform));
                }
            }
        });
    }

private:
    BitEngine::Transform3DProcessor *t3dp;

    BitEngine::ComponentRef<BitEngine::Camera3DComponent> activeCamera;
};


class PlayerControlSystem : public BitEngine::ComponentProcessor
{
public:
    PlayerControlSystem(BitEngine::EntitySystem* es) : BitEngine::ComponentProcessor(es) {}

    static BitEngine::EntityHandle CreatePlayerTemplate(BitEngine::ResourceLoader* loader, MyGameEntitySystem* es, BitEngine::CommandSystem* cmdSys);

    void update()
    {
        BE_PROFILE_FUNCTION();
        getES()->forAll<PlayerControlComponent>([](BitEngine::ComponentHandle id, PlayerControlComponent& comp) {
            float vel = 2.0f;

            // camT2D->setPosition(x, y);
            BitEngine::Vec2 pos = comp.transform2d->getLocalPosition();
            pos.x += comp.movH * vel;
            pos.y += comp.movV * vel;

            comp.transform2d->setLocalPosition(pos);
            comp.transform2d->setLocalRotation(comp.transform2d->getLocalRotation() + 0.03f);
        });
    }
};

class SpinnerComponent : public BitEngine::Component<SpinnerComponent>
{
public:
    SpinnerComponent() {

    }
    SpinnerComponent(float _speed) : speed(_speed) {

    }

    float speed;
};

class SpinnerSystem : public BitEngine::ComponentProcessor
{
public:
    SpinnerSystem(BitEngine::EntitySystem* m) : BitEngine::ComponentProcessor(m) {

    }
    ~SpinnerSystem() {}

    void FrameMiddle()
    {
        using namespace BitEngine;
        getES()->forEach<Transform2DComponent, SpinnerComponent>(
            [=](ComponentRef<Transform2DComponent> transform, const ComponentRef<SpinnerComponent> spinner)
        {
            transform->setLocalRotation(transform->getLocalRotation() + spinner->speed);
        });
    }
};

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
        glp(this), spr2D(this, loader), spinnerSys(this),
        pcs(this),
        mesh3dSys(this, &t3p)
    {
        using namespace BitEngine;

        //t2p = entityMemory->push<Transform2DProcessor>(this);
        //t3p = entityMemory->push<Transform3DProcessor>(this);
        //cam2Dprocessor = entityMemory->push<Camera2DProcessor>(this, t2p);
        //cam3Dprocessor = entityMemory->push<Camera3DProcessor>(this, t3p);
        //rmp = entityMemory->push<RenderableMeshProcessor>(this);
        //
        //glp = entityMemory->push <GameLogicProcessor>(this);
        //spr2D = entityMemory->push < Sprite2DRenderer>(this, loader);
        //spinnerSys = entityMemory->push < SpinnerSystem>(this);

        /// Pipeline 0
        registerComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameStart);

        registerComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameMiddle);

        registerComponentProcessor(0, &spinnerSys, (ComponentProcessor::processFunc)&SpinnerSystem::FrameMiddle);

        registerComponentProcessor(0, &t2p, (ComponentProcessor::processFunc)&Transform2DProcessor::Process);
        registerComponentProcessor(0, &t3p, (ComponentProcessor::processFunc)&Transform3DProcessor::Process);
        registerComponentProcessor(0, &cam2Dprocessor, (ComponentProcessor::processFunc)&Camera2DProcessor::Process);
        registerComponentProcessor(0, &cam3Dprocessor, (ComponentProcessor::processFunc)&Camera3DProcessor::Process);
        registerComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameEnd);

        registerComponentProcessor(0, &pcs, (ComponentProcessor::processFunc)&PlayerControlSystem::update);
        //InitComponentProcessor(spr2D);
    }

    // Processors
    BitEngine::Transform2DProcessor t2p;
    BitEngine::Transform3DProcessor t3p;
    BitEngine::Camera2DProcessor cam2Dprocessor;
    BitEngine::Camera3DProcessor cam3Dprocessor;
    BitEngine::GameLogicProcessor glp;
    BitEngine::Sprite2DRenderer spr2D;
    PlayerControlSystem pcs;
    SpinnerSystem spinnerSys;
    Mesh3DProcessor mesh3dSys;
};


BitEngine::EntityHandle PlayerControlSystem::CreatePlayerTemplate(BitEngine::ResourceLoader* loader, MyGameEntitySystem* es, BitEngine::CommandSystem* cmdSys)
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