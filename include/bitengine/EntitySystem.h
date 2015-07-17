#pragma once

#include <map>
#include <unordered_map>

#include "TypeDefinition.h"
#include "System.h"

#include "ComponentProcess.h"

#define COMPONENT_TYPE_CAMERA2D	0
#define COMPONENT_TYPE_SPRITE2D	1


namespace BitEngine{

typedef uint32 EntityHandle;

/**
* Entity Handle is fixed and won't change at anytime after creation.
* Component Handle is NOT guaranteed to be fixed during entity life
* and may change it's handle during execution.
* A valid entity component may be acquired with an EntitySystem.getComponentOf() call
* Note that the component returned by EntitySystem.getComponentOf is only guaranteed to be valid
* until a component of the same type is destroyed OR an entity that contains a component of the same
* type is destroyed.
*/
class EntitySystem : public System
{
	public:
		EntitySystem()
			: System("Entity"), unique_id(0)
		{
		}

		bool Init() {
			for (auto& it : process_order){
				if (!it.second->Init()){
					return false;
				}
			}
			return true;
		}

		void Update(){
			for (auto& it : process_order){
				it.second->Process();
			}
		}

		void Shutdown(){
			for (auto& it : process_order){
				delete it.second;
			}
		}

		// Register

		template<typename CompType>
		bool RegisterProcessor(IComponentProcess* cs, int priority)
		{
			ComponentType type = CompType::getComponentType();
			auto it = m_processors.find(type);
			if (it != m_processors.end()){
				return false;
			}

			m_processors[type] = cs;
			process_order.insert( std::pair<int, IComponentProcess*>(priority,cs) );
			return true;
		}


		// Get

		template<typename CompType>
		void getComponents(std::vector<CompType>& v){
			auto it = m_processors.find(CompType::getComponentType());
			if (it != m_processors.end()){
				it.second->getComponents(v);
			}
		}

		template<typename CompType>
		CompType* getComponentOf(EntityHandle entity)
		{
			auto it = entity_components.find(entity);
			if (it == entity_components.end()){
				LOGTO(Warning) << "Entity not found: " << entity << endlog;
				return nullptr;
			}

			ComponentType type = CompType::getComponentType();

			const EntityComponents& ec = it->second;
			auto it2 = ec.find(type);
			if (it2 != ec.end()){
				return static_cast<CompType*>(m_processors[type]->getComponent(it2->second));
			}

			LOGTO(Warning) << "Entity " << entity << " does not contain a component of type " << type << endlog;
			return nullptr;
		}


		// Add

		template<typename CompType>
		bool addComponent(EntityHandle entity)
		{
			std::unordered_map<ComponentType, IComponentProcess*>::iterator it = m_processors.find(CompType::getComponentType());
			if (it != m_processors.end()) // Component System found
			{
				auto ec = entity_components.find(entity);
				if (ec != entity_components.end()) // Entity Found
				{
					EntityComponents& components = ec->second;
					auto comp = components.find(CompType::getComponentType());
					if (comp != components.end()) // Already have this component
						return false;

					// Create new component
					ComponentHandle hdl = it->second->CreateComponent();
					if (hdl == 0) // Failed to create component
						return false;

					components[CompType::getComponentType()] = hdl;
					return true;
				}
			}

			return false;
		}


		// Create

		EntityHandle CreateEntity()
		{
			EntityHandle newHandle = ++unique_id;

			// Create entity with no components
			entity_components[newHandle];

			return newHandle;
		}

		void DestroyEntity(EntityHandle entity)
		{
			auto it = entity_components.find(entity);
			if (it == entity_components.end())
				return;

			EntityComponents& ec = it->second;
			for (auto cp : ec)
			{
				m_processors[cp.first]->DestroyComponent(cp.second);
			}
		}

	private:
		typedef std::map<ComponentType, ComponentHandle> EntityComponents;

		std::unordered_map<ComponentType, IComponentProcess*> m_processors;
		std::unordered_map<EntityHandle, EntityComponents> entity_components;
		EntityHandle unique_id;

		std::multimap<int, IComponentProcess*> process_order;
};

}