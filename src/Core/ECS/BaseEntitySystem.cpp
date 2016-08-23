#include "Core/ECS/BaseEntitySystem.h"

#include "Core/ECS/ComponentProcessor.h"
#include "Core/Assert.h"

namespace BitEngine {
	ComponentType BaseComponent::componentTypeCounter(0);

	bool BaseEntitySystem::Init()
	{
		bool initOk = true;
		m_objBitField = new ObjBitField(static_cast<u16>(m_holders.size()));
		m_objBitField->push();

		for (auto& h : m_holders)
		{
			initOk &= h.second->init();
		}

		m_initialized = initOk;
		return m_initialized;
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

		getEngine()->getMessenger()->dispatch(MsgEntityCreated(newHandle));

		return newHandle;
	}

	void BaseEntitySystem::destroyEntity(EntityHandle entity)
	{
		BE_ASSERT(hasEntity(entity));
		m_toBeDestroyed.emplace_back(entity);
		getEngine()->getMessenger()->dispatch(MsgEntityDestroyed(entity));
	}

	bool BaseEntitySystem::addComponent(EntityHandle entity, ComponentType type)
	{
		BE_ASSERT(hasEntity(entity));
		m_objBitField->set(entity, type);
		return true;
	}

	bool BaseEntitySystem::removeComponent(EntityHandle entity, ComponentType type, ComponentHandle handle)
	{
		BE_ASSERT(hasEntity(entity));
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
