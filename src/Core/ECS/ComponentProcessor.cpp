
#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine {

	BaseComponentHolder::BaseComponentHolder(u32 componentSize, u32 nCompPerPool /*= 100*/)
		: m_componentSize(componentSize), m_nComponentsPerPool(nCompPerPool), m_IDcapacity(nCompPerPool),
		m_IDcurrent(1), m_workingComponents(0), m_pools(), m_byEntity(128, 0)
	{
		m_pools.emplace_back(new char[m_componentSize*m_nComponentsPerPool]); // init first pool
		m_freeSorted = true;
	}

	// Returns the released component
	void BaseComponentHolder::releaseComponentForEntity(EntityHandle entity)
	{
		ComponentHandle comp = m_byEntity[entity];

		releaseComponentID(comp);

		sendDestroyMessage(entity, comp);
	}

	void* BaseComponentHolder::getComponent(ComponentHandle componentID)
	{
		char *ptr = m_pools[componentID / m_nComponentsPerPool];
		return ptr + (componentID % m_nComponentsPerPool) * m_componentSize;
	}

	ComponentHandle BaseComponentHolder::getComponentForEntity(EntityHandle entity)
	{
		if (entity >= m_byEntity.size()) {
			return BE_NO_COMPONENT_HANDLE;
		}

		return m_byEntity[entity];
	}

	const std::vector<ComponentHandle>& BaseComponentHolder::getFreeIDs()
	{
		if (!m_freeSorted) {
			std::sort(m_freeIDs.begin(), m_freeIDs.end());
			m_freeSorted = true;
		}

		return m_freeIDs;
	}

	// resize to be able to contain up to given component id
	void BaseComponentHolder::resize(u32 id)
	{
		while (m_IDcapacity <= id)
		{
			m_pools.emplace_back(new char[m_componentSize*m_nComponentsPerPool]);
			m_byComponent.resize(m_byComponent.size() + m_nComponentsPerPool, 0);
			m_IDcapacity += m_nComponentsPerPool;
		}
	}


	u32 BaseComponentHolder::newComponentID(EntityHandle entity)
	{
		ComponentHandle id = BE_NO_COMPONENT_HANDLE;

		// resize vector
		if (entity >= m_byEntity.size()) {
			m_byEntity.resize(entity + 128, 0);
		}

		// find the new ID
		if (m_freeIDs.empty())
		{
			if (m_IDcurrent <= m_IDcapacity)
				resize(m_IDcapacity * 2);

			id = m_IDcurrent++;

		}
		else
		{
			id = m_freeIDs.back();
			m_freeIDs.pop_back();
		}

		m_byEntity[entity] = id;
		m_byComponent[id] = entity;

		++m_workingComponents;

		return id;
	}


}
