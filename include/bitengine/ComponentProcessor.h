#pragma once

#include "TypeDefinition.h"
#include "Component.h"
#include "ComponentCollection.h"

#include <unordered_map>

namespace BitEngine{

class BaseEntitySystem;

class ComponentProcessor
{
public:
	typedef void (ComponentProcessor::* processFunc)(void);
	virtual ~ComponentProcessor() {}

	virtual bool Init(BaseEntitySystem* es) = 0; // Usually used to register listener to ComponentHolders
	virtual void Stop() = 0; // Usually used to unregister listeners from ComponentHolders

	virtual void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) = 0;
	virtual void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) = 0;
};

class ComponentHolder
{
	public:
		virtual ~ComponentHolder() {}

		void RegisterListener(ComponentProcessor* listener) {
			m_listeners.emplace_back(listener);
		}
		void UnregisterListener(ComponentProcessor* listener) {
			for (auto it = m_listeners.begin(); it != m_listeners.end(); ++it) {
				if (*it == listener) {
					m_listeners.erase(it);
					break;
				}
			}
		}
	
		virtual ComponentHandle CreateComponent(EntityHandle entity) {
			auto it = m_componentByEntity.find(entity);
			if (it == m_componentByEntity.end()) {
				ComponentHandle h = AllocComponent();
				m_componentByEntity.emplace(entity, h);
				ComponentCreated(entity, h);
				return h;
			} else { // Not allowing multiple instances of the same component type
				return it->second;
			}
		}

		// Release component owned by given entity
		// Tell all listeners that the component is been released
		virtual void ReleaseComponentFor(EntityHandle entity)
		{
			auto it = m_componentByEntity.find(entity);
			if (it != m_componentByEntity.end()) 
			{
				m_toDestroy.emplace_back(it->second);
				CallOnDestroyListeners(entity, it->second);

				m_componentByEntity.erase(it);
			}
		}

		// Release component
		virtual void ReleaseComponent(ComponentHandle component)
		{
			EntityHandle entity = getComponent(component)->getEntity();
			auto it = m_componentByEntity.find(entity);
			if (it != m_componentByEntity.end())
			{
				// BitEngine::DebugAssert(it->second == component);

				m_toDestroy.emplace_back(component);
				CallOnDestroyListeners(entity, component);

				m_componentByEntity.erase(it);
			}
		}

		virtual const std::vector<ComponentHandle>& getComponents() const = 0;

		virtual Component* getComponent(ComponentHandle component) = 0;

		Component* getComponentFor(EntityHandle entity)
		{
			auto it = m_componentByEntity.find(entity);
			//if (it != m_componentByEntity.end()) {
			return getComponent(it->second);
			//}
		}

		bool HasComponentOnEntity(EntityHandle entity) {
			return m_componentByEntity.find(entity) != m_componentByEntity.end();
		}

	protected:
		void ComponentCreated(EntityHandle entity, ComponentHandle componentHdl) {
			getComponent(componentHdl)->entity = entity;
			m_componentByEntity[entity] = componentHdl;

			// Tell listeners
			for (ComponentProcessor* l : m_listeners) {
				l->OnComponentCreated(entity, componentType, componentHdl);
			}
		}

	private:
		virtual ComponentHandle AllocComponent() = 0;
		virtual void DeallocComponent(ComponentHandle component) = 0;

		void CallOnDestroyListeners(EntityHandle entity, ComponentHandle component) {
			for (ComponentProcessor* l : m_listeners) {
				l->OnComponentDestroyed(entity, componentType, component);
			}
		}

	private:
		std::vector<ComponentProcessor*> m_listeners;
		std::unordered_map<EntityHandle, ComponentHandle> m_componentByEntity;
		std::vector<ComponentHandle> m_toDestroy;

		friend class BaseEntitySystem;
			ComponentHandle getComponentHandleFor(EntityHandle entity) const
			{
				auto it = m_componentByEntity.find(entity);
				if (it != m_componentByEntity.end()) {
					return it->second;
				}

				return 0;
			}

			// Called on frame end
			void DestroyComponents()
			{
				for (ComponentHandle c : m_toDestroy) {
					DeallocComponent(c);
				}
				m_toDestroy.clear();
			}

			ComponentType componentType; // component ID on entity system
			int globalID; // component type global ID
};

}