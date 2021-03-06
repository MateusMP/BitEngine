#pragma once

#include <array>
#include <map>
#include <unordered_map>
#include <functional>

#include <algorithm>

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Core/Logger.h"

#include "BitEngine/Game/ECS/BaseEntitySystem.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"

namespace BitEngine {

/**
* Entity Handle and Component Handle are fixed and won't change at anytime after creation.
* Type safe interface
*/
class BE_API EntitySystem : public BaseEntitySystem {
public:
    EntitySystem()
    {
    }

    ~EntitySystem()
    {
        BaseEntitySystem::shutdown();
    }

    bool init()
    {
        bool initOk = true;
        initOk &= BaseEntitySystem::Init();
        return initOk;
    }

    template <typename CompClass>
    bool registerComponent(ComponentHolder<CompClass>* holder)
    {
        LOG(EngineLog, BE_LOG_VERBOSE) << "Registering " << typeid(CompClass).name() << " as type " << CompClass::getComponentType();
        static_assert(!std::is_pod<CompClass>::value, "Components should be POD!");

        if (!registerComponentHolder(CompClass::getComponentType(), holder)) {
            delete holder;
            return false;
        }

        return true;
    }

    template <typename CompClass>
    bool registerComponent()
    {
        LOG(EngineLog, BE_LOG_WARNING) << "Using generic ComponentHolder for " << typeid(CompClass).name();
        return registerComponent<CompClass>(new ComponentHolder<CompClass>());
    }

    template <typename CompClass>
    ComponentHolder<CompClass>* getHolder()
    {
        return static_cast<ComponentHolder<CompClass>*>(BaseEntitySystem::getHolder(CompClass::getComponentType()));
    }

    template <typename CompClass>
    const ComponentHolder<CompClass>* getHolder() const
    {
        return static_cast<const ComponentHolder<CompClass>*>(BaseEntitySystem::getHolder(CompClass::getComponentType()));
    }

    // Add component
    template <typename CompClass, typename... Args>
    ComponentRef<CompClass> addComponent(EntityHandle entity, Args... args)
    {
        CompClass* comp = nullptr;
        ComponentHandle compID = BE_NO_COMPONENT_HANDLE;
        ComponentType type = CompClass::getComponentType();

        if (BaseEntitySystem::addComponent(entity, type)) {
            ComponentHolder<CompClass>* holder = getHolder<CompClass>();
            compID = holder->createComponent(entity, comp);
            holder->initializeComponent(comp, args...);
            holder->componentCreatedSignal.emit(MsgComponentCreated<CompClass>{ entity, compID });
        }

        return ComponentRef<CompClass>(entity, compID, comp);
    }

    template <typename CompClass>
    bool RemoveComponent(EntityHandle entity, const ComponentRef<CompClass>& ref)
    {
        return BaseEntitySystem::removeComponent(entity, CompClass::getComponentType(), ref.m_componentID);
    }

    /**
    * Get a ComponentRef for given entity
    *
    * \param entity Entity to get the component from
    * @return Returns true when a component reference was found (and returned in ref).
    */
    template <typename CompClass>
    ComponentRef<CompClass> getComponentRef(EntityHandle entity)
    {
        ComponentHolder<CompClass>* holder = getHolder<CompClass>();
        LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

        ComponentHandle compID = holder->getComponentForEntity(entity);
        CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

        return ComponentRef<CompClass>(entity, compID, comp);
    }

    template <typename T>
    struct identity {
        typedef T&& type;
    };

    template <typename Base, typename... ContainComps>
    using Logic = std::function<void(ComponentRef<Base>, ComponentRef<ContainComps>...)>;

    template <typename Base, typename... ContainComps>
    void forEach(typename identity<Logic<Base, ContainComps...> >::type f)
    {
        // TODO: otimize calls of getComponentRefE with this:
        // std::tuple<ComponentHolder<ContainComps>*... > holders = std::make_tuple{ getHolder<ContainComps>()... };

        ComponentType types[] = { ContainComps::getComponentType()... };

        ComponentHolder<Base>* holder = getHolder<Base>();
        LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
        const auto& freeIDs = holder->getFreeIDs(); // sorted ids
        const auto& allComponents = holder->getAllComponents();
        u32 curFree = 0;

        u32 validComponents = holder->getNumValidComponents();

        BitMask componentMask{ 0 };
        for (ComponentType t : types) {
            componentMask.b64 |= 1ull << t;
        }

        // loop through base components searching for matching pairs on Args types
        u32 freeIds = freeIDs.size();
        for (u32 compID = 1; compID <= validComponents; ++compID) {
            const EntityHandle entity = allComponents[compID];

            if ((curFree < freeIds && freeIDs[curFree] != compID) || curFree >= freeIds) {
                // Test to see if this entity has all needed components
                if ((m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64) {
                    Base* comp = holder->getComponent(compID);
                    f(ComponentRef<Base>(entity, compID, comp), getComponentRefE<ContainComps>(entity)...); // TODO: avoid getHolder<>() every time
                }
            }
            else {
                ++curFree;
                ++validComponents;
            }
        }
    }

    /*template<typename Caller, typename Base, typename ... ContainComps>
    void forEach(Caller& caller, typename identity<std::function<void(Caller&, EntityHandle, Base&, ContainComps&...)>>::type f)
    {
        ComponentType types[] = { ContainComps::getComponentType()... };

        ComponentHolder<Base>* holder = getHolder<Base>();
        LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
        const auto& freeIDs = holder->getFreeIDs(); // sorted ids
        const auto& allComponents = holder->getAllComponents();
        u32 curFree = 0;

        u32 validComponents = holder->getNumValidComponents();

        BitMask componentMask{0};
        for (ComponentType t : types){
            componentMask.b64 |= 1ull << t;
        }

        // loop through base components searching for matching pairs on Args types
        for (u32 compID = 1; compID <= validComponents; ++compID)
        {
            const EntityHandle entity = allComponents[compID];

            if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
            {
                // Test to see if this entity has all needed components
                if ( (m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64 )
                {
                    Base* comp = holder->getComponent(compID);

                    f(caller, entity, *comp, getComponentRefE<ContainComps>(entity)...); // TODO: avoid getHolder<>() every time
                }
            }
            else
            {
                ++curFree;
                ++validComponents;
            }
        }
    }*/

    template <typename CompClass>
    void forAll(typename identity<std::function<void(ComponentHandle, CompClass&)> >::type f)
    {
        ComponentHolder<CompClass>* holder = getHolder<CompClass>();
        BE_ASSERT(holder != nullptr);

        const auto& freeIDs = holder->getFreeIDs(); // sorted ids
        u32 curFree = 0;

        u32 validComponents = holder->getNumValidComponents();
        for (u32 compID = 1; compID <= validComponents; ++compID) {
            if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size()) {
                CompClass* comp = holder->getComponent(compID);
                f(compID, *comp);
            }
            else {
                ++curFree;
                ++validComponents;
            }
        }
    }

    template <typename Caller, typename CompClass>
    void forAll(Caller& caller, typename identity<std::function<void(Caller&, ComponentHandle, CompClass&)> >::type f)
    {
        ComponentHolder<CompClass>* holder = getHolder<CompClass>();
        LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

        const auto& freeIDs = holder->getFreeIDs(); // sorted ids
        u32 curFree = 0;

        u32 validComponents = holder->getNumValidComponents();
        for (u32 compID = 1; compID <= validComponents; ++compID) {
            if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size()) {
                CompClass* comp = holder->getComponent(compID);
                f(caller, compID, *comp);
            }
            else {
                ++curFree;
                ++validComponents;
            }
        }
    }

    template <typename CompClass>
    u32 getNumberOfValidComponents() const
    {
        return getHolder<CompClass>()->getNumValidComponents();
    }

private:
    template <typename CompClass>
    ComponentRef<CompClass> getComponentRefE(EntityHandle entity)
    {
        ComponentHolder<CompClass>* holder = getHolder<CompClass>();
        LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

        ComponentHandle compID = holder->getComponentForEntity(entity);
        CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

        return ComponentRef<CompClass>(entity, compID, comp);
    }
};
}
