#pragma once

#include <vector>

#include "BitEngine/Common/MathUtils.h"

#include "BitEngine/Game/ECS/Component.h"
#include "BitEngine/Game/ECS/EntitySystem.h"

namespace BitEngine {

class CollisionLogic {
    virtual ~CollisionLogic() {}
    virtual void OnCollision() = 0;
};

class BE_API GameLogic {
public:
    GameLogic()
        : m_myEntity(0)
        , e_sys(nullptr)
    {
    }
    virtual ~GameLogic() {}

    enum RunEvents : u16 {
        EFrameStart = 1,
        EFrameMiddle = 2,
        EFrameEnd = 4,

        EALL = EFrameStart | EFrameMiddle | EFrameEnd
    };

    virtual RunEvents getRunEvents() = 0;

    virtual bool init() { return true; }

    virtual void frameStart() {}
    virtual void frameMiddle() {}
    virtual void frameEnd() {}

    virtual void end() {}

    template <typename CompClass>
    ComponentRef<CompClass> getComponent()
    {
        return e_sys->getComponentRef<CompClass>(m_myEntity);
    }

private:
    friend class GameLogicComponent;
    EntityHandle m_myEntity;
    EntitySystem* e_sys;
};

class GameLogicComponent : public Component<GameLogicComponent> {
public:
    GameLogicComponent()
        : m_entity(0)
        , e_sys(nullptr)
    {
    }
    GameLogicComponent(EntityHandle ent)
        : m_entity(ent)
        , e_sys(nullptr)
    {
    }

    ~GameLogicComponent()
    {
    }

    void addLogicPiece(GameLogic* logic)
    {
        m_gamelogics.emplace_back(logic);
        logic->m_myEntity = m_entity;
        logic->e_sys = e_sys;
    }

    EntityHandle getEntity() const
    {
        return m_entity;
    }

    // std::vector<GameLogic*>& getLogics() { return m_gamelogics; }

private:
    friend class GameLogicProcessor;
    EntityHandle m_entity;
    EntitySystem* e_sys;
    std::vector<GameLogic*> m_gamelogics;
};
}
