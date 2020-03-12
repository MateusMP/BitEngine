#pragma once

#include "BitEngine/Game/ECS/LogicScript.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"

namespace BitEngine {

class BE_API GameLogicProcessor : public ComponentProcessor,
    public Messenger< MsgComponentCreated<GameLogicComponent>>::ScopedSubscription,
    public Messenger< MsgComponentDestroyed<GameLogicComponent>>::ScopedSubscription
{
public:
    GameLogicProcessor(EntitySystem* m);
    virtual ~GameLogicProcessor();
    
    void FrameStart();
    void FrameMiddle();
    void FrameEnd();

    void onMessage(const MsgComponentCreated<GameLogicComponent>& msg);
    void onMessage(const MsgComponentDestroyed<GameLogicComponent>& msg);

private:
    /// Processor
    void removeFrom(GameLogic* l, std::vector<GameLogic*>& vec);

private:
    /// Processor
    ComponentType GameLogicComponentType;
    ComponentHolder<GameLogicComponent>* gameLogicHolder;

    std::vector<ComponentHandle> m_notInitialized;

    std::vector<GameLogic*> m_onFrameStart;
    std::vector<GameLogic*> m_onFrameMiddle;
    std::vector<GameLogic*> m_onFrameEnd;
};

}