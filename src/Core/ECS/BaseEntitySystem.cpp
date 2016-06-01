#include "Core/ECS/BaseEntitySystem.h"

#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine {
	ComponentType BaseComponent::componentTypeCounter(0);

	bool BaseEntitySystem::Init()
	{
		m_objBitField = new ObjBitField(static_cast<uint16>(m_holders.size()));

		for (auto& h : m_holders)
		{
			h.second->setMessenger(getMessenger());
		}

		m_initialized = true;

		return true;
	}

	EntityHandle BaseEntitySystem::createEntity()
	{
		EntityHandle newHandle;
		if (m_freeEntities.empty()) 
		{
			newHandle = m_entities.size();
			m_entities.emplace_back(newHandle);
			m_objBitField->push();
		}
		else 
		{
			newHandle = m_freeEntities.back();
			m_entities[newHandle] = newHandle;
			m_objBitField->unsetAll(newHandle);
		}

		getMessenger()->SendMessage(MsgEntityCreated(newHandle));

		return newHandle;
	}

	void BaseEntitySystem::destroyEntity(EntityHandle entity)
	{
		if (!hasEntity(entity))
			return;

		m_toBeDestroyed.emplace_back(entity);

		getMessenger()->SendMessage(MsgEntityDestroyed(entity));
	}

	bool BaseEntitySystem::addComponent(EntityHandle entity, ComponentType type)
	{
		if (!hasEntity(entity))
		{
			LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to attach component of type " << type << " to unknown entity: " << entity << "!";
			return false;
		}

		m_objBitField->set(entity, type);

		return true;
	}

	bool BaseEntitySystem::removeComponent(EntityHandle entity, ComponentType type, ComponentHandle handle)
	{
		if (!hasEntity(entity))
		{
			LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to remove component of type " << type << " with handle: " << handle << " from entity " << entity << "!";
			return false;
		}

		m_holders[type]->releaseComponentID(handle);

		return true;
	}

	void BaseEntitySystem::frameFinished()
	{
		for (EntityHandle entity : m_toBeDestroyed)
		{
			for (auto& h : m_holders) 
			{
				if (m_objBitField->test(entity, h.first)) 
				{
					h.second->releaseComponentForEntity(entity);
				}
			}
			m_objBitField->unsetAll(entity);

			m_entities[entity] = 0;
			m_freeEntities.emplace_back(entity);
		}

		m_freeEntities.clear();
	}

}
