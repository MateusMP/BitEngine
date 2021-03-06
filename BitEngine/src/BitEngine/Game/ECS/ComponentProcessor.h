#pragma once

#include <unordered_map>
#include <algorithm>

#include "BitEngine/Core/api.h"
#include "BitEngine/Common/TypeDefinition.h"

#include "BitEngine/Core/Messenger.h"
#include "BitEngine/Game/ECS/Component.h"

namespace BitEngine {

class EntitySystem;
template <typename CompClass>
class ComponentRef;

template <typename CompClass>
struct MsgComponentCreated {
    EntityHandle entity;
    ComponentHandle component;
};

template <typename CompClass>
struct MsgComponentDestroyed {
    EntityHandle entity;
    ComponentHandle component;
};

struct MsgEntityCreated {
    EntityHandle entity;
};

struct MsgEntityDestroyed {
    EntityHandle entity;
};

class ComponentProcessor {
    friend class EntitySystem;

public:
    ComponentProcessor(EntitySystem* m)
        : m_es(m)
    {
    }

    typedef void (ComponentProcessor::*processFunc)(void);
    virtual ~ComponentProcessor() {}

protected:
    EntitySystem* getES() { return m_es; }

    template <typename CompClass>
    inline static ComponentHandle getComponentHandle(const ComponentRef<CompClass>& ref)
    {
        return ref.m_componentID;
    }

private:
    EntitySystem* m_es;
};

class BaseComponentHolder {
    friend class BaseEntitySystem;

public:
    BaseComponentHolder(u32 componentSize, u32 nCompPerPool = 128);
    virtual ~BaseComponentHolder();

    virtual bool init() = 0;

    // Returns the released component
    void releaseComponentForEntity(EntityHandle entity);

    // Returns the component pointer
    void* getComponent(ComponentHandle componentID);

    // Returns the component handle for given entity
    // BE_NO_COMPONENT_HANDLE if there is no such entity/component
    ComponentHandle getComponentForEntity(EntityHandle entity);

    inline EntityHandle getEntityForComponent(ComponentHandle component)
    {
        return m_byComponent[component];
    }

    // Return all ids that were freed
    const std::vector<ComponentHandle>& getFreeIDs();

    // Get all valid component handles
    inline const std::vector<EntityHandle>& getAllComponents()
    {
        return m_byComponent;
    }

    // Return the number of valid components
    inline u32 getNumValidComponents() const
    {
        return m_workingComponents;
    }

    // Resize to be able to contain up to given component id
    void resize(u32 id);

    template <typename CompClass>
    ComponentHandle createComponent(EntityHandle entity, CompClass*& outPtr)
    {
        u32 id = newComponentID(entity);
        outPtr = static_cast<CompClass*>(BaseComponentHolder::getComponent(id));
        return id;
    }

protected:
    virtual void sendDestroyMessage(EntityHandle entity, ComponentHandle component) = 0;

    u32 newComponentID(EntityHandle entity);

private:
    inline void releaseComponentID(ComponentHandle componentID)
    {
        if (!m_freeIDs.empty()) {
            if (m_freeIDs.back() > componentID)
                m_freeSorted = false;
        }

        m_freeIDs.emplace_back(componentID);

        m_byEntity[m_byComponent[componentID]] = BE_NO_COMPONENT_HANDLE;
        m_byComponent[componentID] = 0;

        --m_workingComponents;
    }

protected:
    const u32 m_componentSize;
    const u32 m_nComponentsPerPool;

    u32 m_IDcapacity;
    u32 m_IDcurrent;
    u32 m_workingComponents;
    std::vector<char*> m_pools;
    std::vector<ComponentHandle> m_freeIDs;
    std::vector<EntityHandle> m_byComponent; // given component get the entity
    std::vector<ComponentHandle> m_byEntity; // given entity get the component
    bool m_freeSorted;
};

template <typename CompClass>
class ComponentHolder : public BaseComponentHolder {
    friend class EntitySystem;

public:
    ComponentHolder(u32 componentSize = sizeof(CompClass))
        : BaseComponentHolder(componentSize)
    {
    }

    virtual bool init() override { return true; }

    CompClass* getComponent(ComponentHandle componentID)
    {
        return static_cast<CompClass*>(BaseComponentHolder::getComponent(componentID));
    }

    Messenger<MsgComponentCreated<CompClass> > componentCreatedSignal;
    Messenger<MsgComponentDestroyed<CompClass> > componentDestroyedSignal;

protected:
    template <typename... Args>
    void initializeComponent(CompClass* outPtr, Args... args)
    {
        new (outPtr) CompClass(args...);
    }

    void sendDestroyMessage(EntityHandle entity, ComponentHandle component) override
    {
        componentDestroyedSignal.emit(MsgComponentDestroyed<CompClass>{ entity, component });
    }
};

template <typename CompClass>
class BE_API ComponentRef {
    // Let processors access internal data from Component References
    friend class ComponentProcessor;

public:
    ComponentRef()
        : m_entity(0)
        , m_componentID(0)
        , m_component(nullptr)
    {
    }
    ComponentRef(const ComponentRef& h)
        : m_entity(h.m_entity)
        , m_componentID(h.m_componentID)
        , m_component(h.m_component)
    {
    }
    ComponentRef(ComponentRef&& h) noexcept
        : m_entity(h.m_entity),
          m_componentID(h.m_componentID),
          m_component(h.m_component)
    {
    }
    ComponentRef(EntityHandle entity, ComponentHandle componentID, CompClass* component) noexcept
        : m_entity(entity),
          m_componentID(componentID),
          m_component(component)
    {
    }

    ComponentRef& operator=(const ComponentRef& h)
    {
        m_entity = h.m_entity;
        m_componentID = h.m_componentID;
        m_component = h.m_component;
        return *this;
    }

    operator CompClass*()
    {
        return m_component;
    }

    CompClass* operator->()
    {
        return m_component;
    }

    const CompClass* operator->() const
    {
        return m_component;
    }

    bool isValid() const
    {
        return m_entity != 0 && m_componentID != 0 && m_component != nullptr;
    }

    ComponentHandle getComponentID() const
    {
        return m_componentID;
    }

    CompClass& ref()
    {
        return *m_component;
    }

    const CompClass& ref() const
    {
        return *m_component;
    }

private:
    EntityHandle m_entity;
    ComponentHandle m_componentID;
    CompClass* m_component;
};
}
