#pragma once

#include <unordered_map>
#include <algorithm>

#include "Common/TypeDefinition.h"

#include "Core/Messenger.h"
#include "Core/ECS/Component.h"

namespace BitEngine{

class EntitySystem;

class ComponentCreated : Message<ComponentCreated>
{
	public:
		EntityHandle entity;
		ComponentHandle component;
		ComponentType componentType;
};

class ComponentDestroyed : Message<ComponentDestroyed>
{
	public:
		EntityHandle entity;
		ComponentHandle component;
		ComponentType componentType;
};

class EntityCreated : Message<ComponentDestroyed>
{
	public:
		EntityHandle entity;
};

class ComponentProcessor : public MessengerEndpoint
{
	friend class EntitySystem;
	public:
		typedef void (ComponentProcessor::* processFunc)(void);
		virtual ~ComponentProcessor() {}

		virtual bool Init() = 0; // Usually used to register listener to ComponentHolders
		virtual void Stop() = 0; // Usually used to unregister listeners from ComponentHolders

		virtual void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) = 0;
		virtual void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) = 0;

	protected:
		EntitySystem* getES() { return m_es; }

	private:
		EntitySystem* m_es;

};

class BaseComponentHolder
{
	friend class BaseEntitySystem;
	public:
		BaseComponentHolder(uint32 componentSize, uint32 nCompPerPool=100)
			: m_componentSize(componentSize), m_nComponentsPerPool(nCompPerPool), m_IDcapacity(nCompPerPool),
				m_IDcurrent(1), m_pools(16), m_byEntity(128,0)
		{
			m_pools.emplace_back(new char[m_componentSize*m_nComponentsPerPool]); // init first pool
			m_freeSorted = true;
		}

		virtual ~BaseComponentHolder(){}

		uint32 newComponentID(EntityHandle entity)
		{
			ComponentHandle id = NO_COMPONENT_HANDLE;

			// resize vector
			if (entity >= m_byEntity.size()) {
				m_byEntity.resize(entity + 128, 0);
			}

			// find the new ID
			if (m_freeIDs.empty())
			{
				if (m_IDcurrent >= m_IDcapacity)
					resize(m_IDcapacity*2);

				id = m_IDcurrent++;
			}
			else
			{
				id = m_freeIDs.back();
				m_freeIDs.pop_back();
			}
			
			m_byEntity[entity] = id;
			m_byComponent[id] = entity;
			return id;
		}

		inline void releaseComponentID(ComponentHandle componentID)
		{
			if (!m_freeIDs.empty()) {
				if (m_freeIDs.back() > componentID)
					m_freeSorted = false;
			}

			m_freeIDs.emplace_back(componentID);

			m_byEntity[m_byComponent[componentID]] = NO_COMPONENT_HANDLE;
			m_byComponent[componentID] = 0;
		}

		void releaseComponentForEntity(EntityHandle entity)
		{
			ComponentHandle comp = m_byEntity[entity];

			releaseComponentID(comp);
		}

		void* getComponent(ComponentHandle componentID)
		{
			return &m_pools[componentID / m_nComponentsPerPool][ (componentID % m_nComponentsPerPool) * m_componentSize];
		}

		ComponentHandle getComponentForEntity(EntityHandle entity)
		{
			if (entity >= m_byEntity.size())
				m_byEntity.resize(entity + 128, 0);

			return m_byEntity[entity];
		}

		EntityHandle getEntityForComponent(ComponentHandle component)
		{
			return m_byComponent[component];
		}

		const std::vector<ComponentHandle>& getFreeIDs() {
			if (!m_freeSorted) {
				std::sort(m_freeIDs.begin(), m_freeIDs.end());
				m_freeSorted = true;
			}

			return m_freeIDs;
		}

		const std::vector<EntityHandle>& getAllComponents() {
			return m_byComponent;
		}

		uint32 getNumValidComponents() {
			return m_IDcurrent - m_freeIDs.size();
		}

		// resize to be able to contain up to given component id
		void resize(uint32 id)
		{
			while (m_IDcapacity <= id)
			{
				m_pools.emplace_back(new char[m_componentSize*m_nComponentsPerPool]);
				m_byComponent.resize(m_byComponent.size() + m_nComponentsPerPool, 0);
				m_IDcapacity += m_nComponentsPerPool;
			}
		}

	protected:
		const uint32 m_componentSize;
		const uint32 m_nComponentsPerPool;

		uint32 m_IDcapacity;
		uint32 m_IDcurrent;
		std::vector<char*> m_pools;
		std::vector<ComponentHandle> m_freeIDs;
		std::vector<EntityHandle> m_byComponent; // given component get the entity
		std::vector<ComponentHandle> m_byEntity; // given entity get the component
		bool m_freeSorted;
};

template <typename T>
class ComponentHolder : public BaseComponentHolder
{
	public:
		T* getComponent(ComponentHandle componentID)
		{
			return static_cast<T*>(BaseComponentHolder::getComponent(componentID));
		}

		template<typename ... Args>
		ComponentHandle createComponent(T*& outPtr, Args ...args)
		{
			uint32 id = newComponentID();
			T* ptr = static_cast<T*>(BaseComponentHolder::getComponent(id));

			new (ptr) T(args...);

			return ptr;
		}
};

template<typename CompClass>
class ComponentRef
{
	public:
		ComponentRef(const ComponentRef& h)
			: m_entity(h.m_entity), m_componentID(h.m_componentID), m_es(h.m_es), m_component(h.m_component)
		{}
		ComponentRef(ComponentRef&& h) noexcept
			: m_entity(h.m_entity), m_componentID(h.m_componentID), m_es(h.m_es), m_component(h.m_component)
		{}

		ComponentRef(EntityHandle entity, ComponentHandle componentID, EntitySystem* entitySys, CompClass* component) noexcept
			: m_entity(entity), m_componentID(componentID), m_es(entitySys), m_component(component)
		{}

		operator CompClass*() {
			return m_component;
		}

		CompClass* operator->() {
			return m_component;
		}

		const CompClass* operator ->() const {
			return m_component;
		}

	private:
		EntityHandle m_entity;
		ComponentHandle m_componentID;
		EntitySystem* m_es;
		CompClass* m_component;
};


}
