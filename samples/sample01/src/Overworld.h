#pragma once

#include <BitEngine/Core/Input.h>
#include <BitEngine/Core/CommandSystem.h>

#include "MyGameEntitySystem.h"

class PlayerController;

class PlayerCamera
{
public:
    bool Create(MyGameEntitySystem* esys)
    {
        entity = esys->createEntity();
        transform = esys->addComponent<BitEngine::Transform3DComponent>(entity);
        camera = esys->addComponent<BitEngine::Camera3DComponent>(entity);

        transform->setPosition(0, 0, 250);

        if (!transform.isValid()) return false;
        if (!camera.isValid()) return false;

        return true;
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

class Character
{
public:
    Character(u32 _id)
        : id(_id)
    {}

    virtual void Create(MyGameEntitySystem* esys)
    {
        entity = esys->createEntity();
        transform = esys->addComponent<BitEngine::Transform3DComponent>(entity);
        renderable = esys->addComponent<BitEngine::RenderableMeshComponent>(entity);
    }

    void setPosition(const glm::vec3& pos) {
        transform->setPosition(pos);
    }

    u32 getID() const {
        return id;
    }

    BitEngine::EntityHandle getEntity() const {
        return entity;
    }

    BitEngine::ComponentRef<BitEngine::Transform3DComponent>& getTransform() {
        return transform;
    }

protected:
    BitEngine::EntityHandle entity;
    BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;
    BitEngine::ComponentRef<BitEngine::RenderableMeshComponent> renderable;

private:
    u32 id;
};


class Player
{
public:
    Player(const std::string& nick, u32 netid)
        : m_nickname(nick), m_netID(netid), m_character(nullptr)
    {
    }

    const std::string& getNickname() const {
        return m_nickname;
    }

    u32 getNetID() const {
        return m_netID;
    }

    void setCharacter(Character* character) {
        m_character = character;
    }

private:
    std::string m_nickname;
    u32 m_netID;

    Character* m_character;

};

class PlayerController : public BitEngine::GameLogic,
    BitEngine::Messenger< BitEngine::CommandSystem::MsgCommandInput>::ScopedSubscription
{
public:
    PlayerController(MainMemory* m, Player* p, PlayerCamera* cam)
        : GameLogic(), player(p), camera(cam),
        BitEngine::Messenger< BitEngine::CommandSystem::MsgCommandInput>::ScopedSubscription(m->commandSystem->commandSignal, &PlayerController::onMessage, this)
    {

    }

    RunEvents getRunEvents() override {
        return RunEvents::EALL;
    }

    bool init() override
    {
        transform = getComponent<BitEngine::Transform3DComponent>();
        if (!transform.isValid()) {
            return false;
        }

        return true;
    }

    void onMessage(const BitEngine::CommandSystem::MsgCommandInput& msg)
    {
        switch (msg.commandID) {
        case RIGHT:
            moveDirection.x = 1.0f;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                moveDirection.x = 0.0f;
            break;
        case LEFT:
            moveDirection.x = -1.0f;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                moveDirection.x = 0.0f;
            break;
        case UP:
            moveDirection.z = -1.0f;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                moveDirection.z = 0.0f;
            break;
        case DOWN:
            moveDirection.z = 1.0f;
            if (msg.action.fromButton == BitEngine::KeyAction::RELEASE)
                moveDirection.z = 0.0f;
            break;
        }

        if (glm::length(moveDirection) > 0)
            moveDirection = glm::normalize(moveDirection);
    }

    void frameStart() override {
        const auto ctransform = transform;

        glm::vec3 velocity(moveDirection.x, 0, moveDirection.z);

        transform->setPosition(ctransform->getPosition() + velocity);
        playerLookDirection = ctransform->getPosition() + BitEngine::Transform3DComponent::FORWARD;

        camera->setLookAt(playerLookDirection);
    };

    void frameMiddle() override {

    };

    void frameEnd() override {

    };

    void end() override {
    }

    PlayerCamera* getCamera() const {
        return camera;
    }

    const glm::vec3& getPlayerLookDirection() const {
        return playerLookDirection;
    }

private:
    Player* player;
    PlayerCamera* camera;
    BitEngine::ComponentRef<BitEngine::Transform3DComponent> transform;

    float maxSpeed = 10;
    glm::vec3 moveDirection;
    glm::vec3 playerLookDirection;
};


class GameWorld
{
public:
    GameWorld(MainMemory* memory, MyGameEntitySystem* es)
        : m_ES(es), memory(memory)
    {
    }
    ~GameWorld() {
    };

    // Prepare all resources
    virtual bool init() {
        return true;
    }
    // Releases all resources
    virtual void shutdown() {

    }

    void setActiveCamera(const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& camera)
    {
        activeCamera = camera;
    }

    const BitEngine::ComponentRef<BitEngine::Camera3DComponent> getActiveCamera() const
    {
        return activeCamera;
    }

    virtual int addPlayer(Player* player)
    {
        m_players.push_back(player);
        return true;
    }

private:
    MainMemory *memory;
    std::vector<Player*> m_players;

    MyGameEntitySystem * m_ES;
    BitEngine::ComponentRef<BitEngine::Camera3DComponent> activeCamera;

};

