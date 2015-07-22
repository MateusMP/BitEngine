#pragma once

#include <map>
#include <unordered_map>

#include <algorithm>

#include "EngineLoggers.h"
#include "TypeDefinition.h"
#include "System.h"

#include "ComponentProcessor.h"
#include "ResourceSystem.h"

namespace BitEngine{

enum UpdateEvent : uint16{
	BeginFrame = 0x01,
	MidFrame = 0x02,
	EndFrame = 0x04,

	TOTAL = 3,

	ALL = BeginFrame | MidFrame | EndFrame
};

template<typename CompClass>
class ComponentRef
{
	public:
		ComponentRef() 
			: handle(0), holder(nullptr)
		{}

		ComponentRef(ComponentHandle h, ComponentHolderProcessor* p)
			: handle(h), holder(p)
		{}
		/*
		const CompClass* operator -> () const{
			printf("Calling CONST operator\n");
			return (const CompClass*)holder->getComponent(handle);
		}
		*/
		CompClass* operator -> () {
			// printf("Calling NORMAL operator\n");
			return (CompClass*) holder->getComponent(handle);
		}

		ComponentHandle getComponentHandle(){
			return handle;
		}
		

	private:
		friend class EntitySystem;
		ComponentHandle handle;
		ComponentHolderProcessor* holder;
};

/**
* Entity Handle and Component Handle are fixed and won't change at anytime after creation.
* 
*/
class EntitySystem : public System
{
	public:
		EntitySystem(ResourceSystem* resources);

		bool Init();
		void Update();
		void Shutdown();

		// Register

		template<typename CompClass>
		bool RegisterComponentHolderProcessor(ComponentHolderProcessor* cs, int priority, UpdateEvent updateEvent)
		{
			ComponentType type = CompClass::getComponentType();
			if (m_dataHolderProcessors.size() <= (uint32)type)
			{
				m_dataHolderProcessors.resize((uint32)type + 1, nullDHP);
			}
			else if (m_dataHolderProcessors[type].processor != nullptr)
			{
				LOGTO(Warning) << "EntitySystem: Trying to register two main processors for the component type: " << type << endlog;
				return false;
			}

			m_dataHolderProcessors[type] = DHP(cs, type);

			process_order[0].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::BeginFrame)
				process_order[1].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::MidFrame)
				process_order[2].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::EndFrame)
				process_order[3].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));

			ordered = false;

			return true;
		}

		bool RegisterDataProcessor(ComponentProcessor* cs, int priority, UpdateEvent updateEvent)
		{
			process_order[0].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::BeginFrame)
				process_order[1].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::MidFrame)
				process_order[2].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));
			if (updateEvent & UpdateEvent::EndFrame)
				process_order[3].emplace_back(std::pair<int, ComponentProcessor*>(priority, cs));

			ordered = false;

			return true;
		}


		bool isComponentOfTypeValid(ComponentType type){
			return m_dataHolderProcessors.size() > type;
		}

		bool hasEntity(EntityHandle entity){
			return m_entities.size() > entity && m_entities[entity] == entity;
		}
		
		// Get
		
		template<typename CompType>
		CompType* getComponentUnsafe(EntityHandle entity)
		{
			// Verify if component type is valid
			ComponentType type = CompType::getComponentType();
			if (!isComponentOfTypeValid(type)){
				LOGTO(Warning) << "EntitySystem: Unregistered type: " << type << endlog;
				return nullptr;
			}

			DHP& dhp = m_dataHolderProcessors[type];

			// Verify if such entity exists for given system
			return (CompType*) dhp.getComponentRefFor(entity);
		}

		template<typename CompClass>
		bool getComponentRef(EntityHandle entity, ComponentRef<CompClass>& ref)
		{
			// Verify if component type is valid
			ComponentType type = CompClass::getComponentType();
			if (!isComponentOfTypeValid(type)){
				LOGTO(Warning) << "EntitySystem: Unregistered type: " << type << endlog;
				return nullptr;
			}

			DHP& dhp = m_dataHolderProcessors[type];

			if (dhp.hasEntity(entity))
			{
				ref.holder = dhp.processor;
				ref.handle = dhp.getComponentFor(entity);
				return true;
			}

			return false;
		}

		// Search
		template<typename BaseSearchCompClass, typename... Having>
		void findAllTuples(const std::vector<ComponentHandle> &search, std::vector<ComponentHandle>& answer, std::vector<uint32>& matchSearchIndices)
		{
			int nTypes = sizeof...(Having);
			ComponentType typeSearch = BaseSearchCompClass::getComponentType();

			if (!isComponentOfTypeValid(typeSearch)){
				matchSearchIndices.clear(); // nothing found!
				LOGTO(Warning) << "EntitySystem: Trying to search for invalid type: " << typeSearch << endlog;
				return;
			}

			// Find all entities IDs of search
			int i;
			for (size_t k = 0; k < search.size(); ++k)
			{
				const ComponentHandle hdl = search[k];
				EntityHandle entity = m_dataHolderProcessors[typeSearch].getEntityFor(hdl);
				ComponentHandle componentsFound[] = { getComponentFromEntity<Having>(entity)... };

				// printf("Entity: %d\n", entity);

				for (i = 0; i < nTypes; ++i){
					// printf("%d ; ", componentsFound[i]);
					if (componentsFound[i] == 0) break;
				}
				// printf("\n");
				if (i == nTypes){
					// printf("Match found: ");
					for (i = 0; i < nTypes; ++i){
						answer.push_back(componentsFound[i]);
						// printf("%d, ", componentsFound[i]);
					}
					// printf("\n");
					matchSearchIndices.push_back(k);
				}
				else{
					// printf("jumping hdl: [%d]%d\n", k, hdl);
				}

				/*ComponentHandle x = getComponentFromEntity<Having>(entity);
				if ((x != 0))
				{
					printf("X: %d\n", x);
					// answer.push_back(x)...;
				}*/
			}

		}


		// Add

		template<typename CompClass>
		bool addComponent(EntityHandle entity, ComponentRef<CompClass>& ref)
		{
			ComponentType type = CompClass::getComponentType();
			if (!isComponentOfTypeValid(type)){
				LOGTO(Warning) << "EntitySystem: Trying to attach component unknown type " << type << " to entity: " << entity << "!" << endlog;
				return false;
			}

			if (!hasEntity(entity)){
				LOGTO(Warning) << "EntitySystem: Trying to attach component of type " << type << " to unknown entity: " << entity << "!" << endlog;
				return false;
			}

			DHP& p = m_dataHolderProcessors[type];
			ComponentHandle hdl = p.createComponentFor(entity);
			if (hdl == 0){
				LOGTO(Warning) << "EntitySystem: Entity " << entity << " already has a component of type: " << type << "!" << endlog;
				return false;
			}

			ref.handle = hdl;
			ref.holder = p.processor;

			return true;
		}

		ResourceSystem* getResourceSystem();

		// Create

		EntityHandle CreateEntity()
		{
			if (m_freeEntities.empty()){
				EntityHandle newHandle = m_entities.size();
				m_entities.emplace_back(newHandle);

				return newHandle;
			}
			
			EntityHandle newHandle = m_freeEntities.back();
			m_entities[newHandle] = newHandle;

			return newHandle;
		}

		void DestroyEntity(EntityHandle entity)
		{
			if (!hasEntity(entity))
				return;

			m_freeEntities.emplace_back(entity);
			m_entities[entity] = 0;

			for (DHP& dhp : m_dataHolderProcessors){
				dhp.destroyComponentFor(entity);
			}
		}


	private:
		template<typename CompType>
		ComponentHandle getComponentFromEntity(EntityHandle entity)
		{
			ComponentType type = CompType::getComponentType();
			return m_dataHolderProcessors[type].getComponentFor(entity);
		}

		void sortProcess();

		// Data Holder Processor
		struct DHP{
			DHP(ComponentHolderProcessor* p, ComponentType t)
				: processor(p), type(t)
			{}

			EntityHandle getEntityFor(ComponentHandle component){
				if (entitiesHandles.size() > component)
					return entitiesHandles[component];
				return 0;
			}

			ComponentHandle getComponentFor(EntityHandle entity){
				if (componentsHandles.size() > entity)
					return componentsHandles[entity];
				return 0;
			}

			void* getComponentRefFor(EntityHandle entity){
				if (hasEntity(entity)){
					return processor->getComponent(componentsHandles[entity]);
				}

				return nullptr;
			}

			ComponentHandle createComponentFor(EntityHandle entity){
				if (hasEntity(entity)){
					return 0;
				}

				ComponentHandle component = processor->CreateComponent(entity);
				if (component == 0)
					return 0;

				if (entitiesHandles.size() <= component){
					entitiesHandles.resize(component+1, 0);
				}
				if (componentsHandles.size() <= entity){
					componentsHandles.resize(entity + 1, 0);
				}

				entitiesHandles[component] = entity;
				componentsHandles[entity] = component;
				LOGTO(Verbose) << "Added component " << component << " of type " << type << " to entity " << entity << endlog;

				return component;
			}

			void destroyComponentFor(EntityHandle entity){
				if (!hasEntity(entity))
					return;

				const ComponentHandle component = componentsHandles[entity];
				processor->DestroyComponent(component);
				entitiesHandles[component] = 0;
				componentsHandles[entity] = 0;
			}

			// True if there is an entity with this component type
			bool hasEntity(EntityHandle entity){
				return componentsHandles.size() > entity && componentsHandles[entity] != 0;
			}

			// True if there is a component with this handle
			bool hasComponent(ComponentHandle handle){
				return entitiesHandles.size() > handle && entitiesHandles[handle] != 0;
			}
			
			ComponentHolderProcessor* processor;
			ComponentType type;

			std::vector<EntityHandle> entitiesHandles; // By componentHdl
			std::vector<ComponentHandle> componentsHandles; // By entityHdl
		};
		DHP nullDHP;
		std::vector< DHP > m_dataHolderProcessors;
		std::vector< EntityHandle > m_entities;
		std::vector< EntityHandle > m_freeEntities;

		bool ordered;
		std::vector< std::pair<int, ComponentProcessor*> > process_order[4];

		ResourceSystem* m_resources;
};

}