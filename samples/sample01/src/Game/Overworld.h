#pragma once

#include <BitEngine/Core/Input.h>
#include <BitEngine/Core/CommandSystem.h>

#include "MyGameEntitySystem.h"

class PlayerCamera
{
public:
    PlayerCamera(MyGameEntitySystem* esys) {
        entity = esys->createEntity();
        transform = esys->addComponent<BitEngine::Transform3DComponent>(entity);
        camera = esys->addComponent<BitEngine::Camera3DComponent>(entity);

        transform->setPosition(0, 0, 250);
    }

    void setLookAt(const glm::vec3& pos) {
        camera->setLookAt(pos);
    }

    BitEngine::ComponentRef<BitEngine::Camera3DComponent>& getCamera() {
        return camera;
    }

    BitEngine::ComponentRef<BitEngine::Transform3DComponent>& getTransform() {
        return transform;
    }

protected:
    BitEngine::EntityHandle entity;
    BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;
    BitEngine::ComponentRef<BitEngine::Camera3DComponent> camera;
};

class GameWorld
{
public:
    GameWorld(MainMemory* memory, MyGameEntitySystem* es)
        : memory(memory), m_ES(es)
    {
    }
    ~GameWorld() {
    };

    void setActiveCamera(const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& camera)
    {
        activeCamera = camera;
    }

    const BitEngine::ComponentRef<BitEngine::Camera3DComponent> getActiveCamera() const
    {
        return activeCamera;
    }

private:
    MainMemory *memory;
    MyGameEntitySystem * m_ES;
    BitEngine::ComponentRef<BitEngine::Camera3DComponent> activeCamera;

};

