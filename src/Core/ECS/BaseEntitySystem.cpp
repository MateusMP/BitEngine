#include "Core/ECS/BaseEntitySystem.h"

#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine {

	bool BaseEntitySystem::RegisterHolder(ComponentHolder* holder, int id, int globalID)
	{
		if (m_dataHolder[id] == nullptr)
		{
			m_dataHolder[id] = holder;
			holder->componentType = id;
			holder->globalID = globalID;
			return true;
		}

		return false;
	}

	EntityHandle BaseEntitySystem::CreateEntity()
	{
		if (m_freeEntities.empty()) {
			EntityHandle newHandle = m_entities.size();
			m_entities.emplace_back(newHandle);

			return newHandle;
		}

		EntityHandle newHandle = m_freeEntities.back();
		m_entities[newHandle] = newHandle;

		return newHandle;
	}

	void BaseEntitySystem::DestroyEntity(EntityHandle entity)
	{
		if (!hasEntity(entity))
			return;

		m_toBeDestroyed.emplace_back(entity);

		for (EntityHandle entity : m_toBeDestroyed)
		{
			for (ComponentHolder* h : m_dataHolder) {
				h->ReleaseComponentFor(entity);
			}
		}
	}

	void BaseEntitySystem::FrameFinished()
	{
		for (ComponentHolder* h : m_dataHolder) {
			h->DestroyComponents();
		}

		for (EntityHandle entity : m_toBeDestroyed){
			m_entities[entity] = 0;
			m_freeEntities.emplace_back(entity);
		}
		m_freeEntities.clear();
	}

}