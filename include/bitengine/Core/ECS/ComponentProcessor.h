#pragma once

#include <unordered_map>
#include <algorithm>

#include "Common/TypeDefinition.h"

#include "Core/Messenger.h"
#include "Core/ECS/Component.h"

namespace BitEngine{

class EntitySystem;
template<typename CompClass> class ComponentRef;

template<typename CompClass>
class MsgComponentCreated : public Message<MsgComponentCreated<CompClass> >
{
	public:
		MsgComponentCreated(EntityHandle entity_, ComponentType componentType_, ComponentHandle component_)
			: entity(entity_), componentType(componentType_), component(component_) {}

		EntityHandle entity;
		ComponentType componentType;
		ComponentHandle component;
};

template<typename CompClass>
class MsgComponentDestroyed : public Message<MsgComponentDestroyed<CompClass> >
{
	public:
		MsgComponentDestroyed(EntityHandle entity_, ComponentType componentType_, ComponentHandle component_)
			: entity(entity_), componentType(componentType_), component(component_) {}

		EntityHandle entity;
		ComponentType componentType;
		ComponentHandle component;
};

class MsgEntityCreated : public Message<MsgEntityCreated>
{
	public:
		MsgEntityCreated(EntityHandle entity_)
			: entity(entity_) {}

		EntityHandle entity;
};

class MsgEntityDestroyed : public Message<MsgEntityDestroyed>
{
public:
	MsgEntityDestroyed(EntityHandle entity_)
		: entity(entity_) {}

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

	protected:
		EntitySystem* getES() { return m_es; }

		template<typename CompClass>
		inline static ComponentHandle getComponentHandle(const ComponentRef<CompClass>& ref) {
			return ref.m_componentID;
		}

	private:
		EntitySystem* m_es;

};

class BaseComponentHolder : public MessengerEndpoint
{
	friend class BaseEntitySystem;
	public:
		BaseComponentHolder(u32 componentSize, u32 nCompPerPool = 128);

		virtual ~BaseComponentHolder(){}

		// Returns the released component
		void releaseComponentForEntity(EntityHandle entity);
		
		// Returns the component pointer
		void* getComponent(ComponentHandle componentID);

		// Returns the component handle for given entity
		// BE_NO_COMPONENT_HANDLE if there is no such entity/component
		ComponentHandle getComponentForEntity(EntityHandle entity);

		inline EntityHandle getEntityForComponent(ComponentHandle component) {
			return m_byComponent[component];
		}

		// Return all ids that were freed
		const std::vector<ComponentHandle>& getFreeIDs();

		// Get all valid component handles
		inline const std::vector<EntityHandle>& getAllComponents() {
			return m_byComponent;
		}

		// Return the number of valid components
		inline u32 getNumValidComponents() const {
			return m_workingComponents;
		}

		// Resize to be able to contain up to given component id
		void resize(u32 id);

	protected:
		virtual void sendDestroyMessage(EntityHandle entity, ComponentHandle component) = 0;

		u32 newComponentID(EntityHandle entity);

	private:
		inline void releaseComponentID(ComponentHandle componentID)
		{
			if (!m_freeIDs.empty()) 
			{
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
class ComponentHolder : public BaseComponentHolder
{
	friend class EntitySystem;
	public:
		ComponentHolder(u32 componentSize = sizeof(CompClass))
			: BaseComponentHolder(componentSize)
		{}

		CompClass* getComponent(ComponentHandle componentID)
		{
			return static_cast<CompClass*>(BaseComponentHolder::getComponent(componentID));
		}

	protected:
		virtual void createdComponent(EntityHandle entity, ComponentHandle compId, CompClass* ptr) {}

	private:
		template<typename ... Args>
		ComponentHandle createComponent(EntityHandle entity, CompClass*& outPtr, Args ...args)
		{
			u32 id = newComponentID(entity);
			outPtr = static_cast<CompClass*>(BaseComponentHolder::getComponent(id));

			new (outPtr) CompClass(args...);

			createdComponent(entity, id, outPtr);
			
			return id;
		}

		void sendDestroyMessage(EntityHandle entity, ComponentHandle component) override
		{
			getMessenger()->SendMessage(MsgComponentDestroyed<CompClass>(entity, CompClass::getComponentType(), component));
		}
};

template<typename CompClass>
class ComponentRef
{
	// Let processors access internal data from Component References
	friend class ComponentProcessor;

	public:
		ComponentRef()
			: m_entity(0), m_componentID(0), m_es(nullptr), m_component(nullptr)
		{}
		ComponentRef(const ComponentRef& h)
			: m_entity(h.m_entity), m_componentID(h.m_componentID), m_es(h.m_es), m_component(h.m_component)
		{}
		ComponentRef(ComponentRef&& h) noexcept
			: m_entity(h.m_entity), m_componentID(h.m_componentID), m_es(h.m_es), m_component(h.m_component)
		{}

		ComponentRef(EntityHandle entity, ComponentHandle componentID, EntitySystem* entitySys, CompClass* component) noexcept
			: m_entity(entity), m_componentID(componentID), m_es(entitySys), m_component(component)
		{}

		ComponentRef& operator =(const ComponentRef& h) {
			m_entity = h.m_entity;
			m_componentID = h.m_componentID;
			m_es = h.m_es;
			m_component = h.m_component;
			return *this;
		}

		operator CompClass*() {
			return m_component;
		}

		CompClass* operator->() {
			return m_component;
		}

		const CompClass* operator ->() const {
			return m_component;
		}

		bool isValid() const {
			return m_entity != 0 && m_componentID != 0 && m_es != nullptr && m_component != nullptr;
		}

		ComponentHandle getComponentID() const {
			return m_componentID;
		}

		CompClass& ref() {
			return *m_component;
		}

		const CompClass& ref() const {
			return *m_component;
		}

	private:
		EntityHandle m_entity;
		ComponentHandle m_componentID;
		EntitySystem* m_es;
		CompClass* m_component;
};


}
