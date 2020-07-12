#pragma once

#include <vector>

#include "BitEngine/Common/BitFieldVector.h"
#include "BitEngine/Game/ECS/Component.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"
#include "BitEngine/Core/Logger.h"

namespace BitEngine {

class BE_API BaseEntitySystem : 
        public Messenger<MsgEntityCreated>, public Messenger<MsgEntityDestroyed>
{
public:
    BaseEntitySystem()
    {
        m_initialized = false;
        m_objBitField = nullptr;
        m_entities.emplace_back(0); // First entity is invalid.
    }

    // After this call, no more component types are allowed to be registered
    // return true if initialized correctly.
    bool Init();

    // Creates a new Entity
    // @return The entity handle
    EntityHandle createEntity();

    // Destroy the entity
    // All components inside this entity will be destroyed too
    // @param entity Entity to be destroyed
    void destroyEntity(EntityHandle entity);

    // Add component of given type for an entity entity
    // @param entity Entity id
    // @param type Component Type id
    bool addComponent(EntityHandle entity, ComponentType type);

    // Remove component of type from an entity
    // @param entity The entity id
    // @param type The component type id
    // @param handle The component handle
    // @return true if component was found and removed
    bool removeComponent(EntityHandle entity, ComponentType type, ComponentHandle handle);

    // Check if given component type is valid inside this BaseEntitySystem
    // @param type The type to check
    // @return true if it is valid
    bool isComponentOfTypeValid(ComponentType type) const {
        return m_holders.find(type) != m_holders.end();
    }

    // Get the component holder for given ComponentType
    // Assumes a valid type is given @see isComponentOfTypeValid()
    // @param The component type
    // @return A raw pointer for the component holder
    inline BaseComponentHolder* getHolder(ComponentType type) {
        return m_holders[type];
    }

    // Get the component holder for given ComponentType.
    // @param The component type
    // @return A const pointer for the component holder.
    inline const BaseComponentHolder* getHolder(ComponentType type) const
    {
        auto it = m_holders.find(type);
        if (it != m_holders.end())
            return it->second;
        return nullptr;
    }

    // Should be called once the Update() is finished, after all Processors have 
    // finished execution for this frame.
    // Releases all memory used by destroyed components/entities
    void destroyPending();

protected:
    // Verify if entity has the component type
    bool hasComponent(EntityHandle entity, ComponentType type)
    {
        return m_objBitField->test(entity, type);
    }

    // Shut down entity system
    void shutdown()
    {
        for (auto& h : m_holders)
        {
            delete h.second;
        }

        delete m_objBitField;
    }

    // Register a component holder for given component type
    bool registerComponentHolder(ComponentType type, BaseComponentHolder* holder)
    {
        if (m_initialized) {
            return false;
        }

        auto it = m_holders.find(type);
        if (it != m_holders.end()) {
            LOG(EngineLog, BE_LOG_ERROR) << "There is another component holder already registered for the type " << type;
            return false;
        }

        m_holders[type] = holder;
        return true;
    }

    // TODO: Make this a DebugAssert?
    inline bool hasEntity(EntityHandle entity) const {
        return m_entities.size() > entity && m_entities[entity] == entity;
    }

    // Member variables
    std::vector< EntityHandle > m_entities;
    std::vector< EntityHandle > m_freeEntities;
    std::vector< EntityHandle > m_toBeDestroyed;
    ObjBitField *m_objBitField;

private:
    std::unordered_map< ComponentType, BaseComponentHolder* > m_holders;

    bool m_initialized;
};

}
