#include "Core/ECS/BaseEntitySystem.h"

#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine {
	ComponentType BaseComponent::componentTypeCounter(0);

	EntityHandle BaseEntitySystem::CreateEntity()
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

	void BaseEntitySystem::DestroyEntity(EntityHandle entity)
	{
		if (!hasEntity(entity))
			return;

		m_toBeDestroyed.emplace_back(entity);

		getMessenger()->SendMessage(MsgEntityDestroyed(entity));
	}

	void BaseEntitySystem::FrameFinished()
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
