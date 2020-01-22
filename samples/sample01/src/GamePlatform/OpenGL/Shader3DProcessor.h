#pragma once

#include <BitEngine/Core/VideoSystem.h>
#include <BitEngine/Core/Graphics/Sprite2D.h>
#include <BitEngine/Core/ECS/EntitySystem.h>
#include <BitEngine/Core/ECS/Transform3DProcessor.h>

#include "Shader3DSimple.h"

class Shader3DProcessor : public BitEngine::ComponentProcessor
{
public:
    Shader3DProcessor(BitEngine::EntitySystem* es, BitEngine::Transform3DProcessor *t3dp_)
        : ComponentProcessor(es), t3dp(t3dp_)
    {
        shader3DSimple = new Shader3DSimple();
        if (!shader3DSimple->Init())
            return;
        shader3DSimpleRenderer = shader3DSimple->CreateRenderer();
        if (shader3DSimpleRenderer == nullptr) {
            LOG(GameLog(), BE_LOG_ERROR) << "Could not create Shader3DSimple renderer";
            return;
        }
    }

    void setActiveCamera(const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& camera)
    {
        activeCamera = camera;
    }

    void Render()
    {
        using namespace BitEngine;

        // Find out the 2D camera

        shader3DSimpleRenderer->Begin();

        // printf("3d To render: %d\n", nObjs);
        /*getES()->forEach<RenderableMeshComponent, Transform3DComponent>(
            [this](ComponentRef<RenderableMeshComponent>& renderable, ComponentRef<Transform3DComponent>&& transform)
        {
            if (renderable->getMesh() && renderable->getMesh()->getType() == Shader3DSimple::GetID())
            {
                shader3DSimpleRenderer->addMesh((Shader3DSimple::Mesh*)renderable->getMesh(),
                    (Shader3DSimple::Material*)renderable->getMaterial(),
                    &(t3dp->getGlobalTransformFor(getComponentHandle(transform))));
            }
        });*/
        shader3DSimpleRenderer->End();

        // Set up shader
        shader3DSimple->LoadProjectionMatrix(activeCamera->getProjection());
        shader3DSimple->LoadViewMatrix(activeCamera->getView());
        shader3DSimple->Bind();

        // Set up GL states
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        // Render
        shader3DSimpleRenderer->Render();
    }

private:
    BitEngine::Transform3DProcessor *t3dp;

    Shader3DSimple* shader3DSimple;
    Shader3DSimple::BatchRenderer* shader3DSimpleRenderer;

    BitEngine::ComponentRef<BitEngine::Camera3DComponent> activeCamera;
};