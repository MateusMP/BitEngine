#pragma once

#include <vector>

#include "Core/System.h"
#include "Core/ECS/Component.h"

namespace BitEngine {

	class ComponentHolder;


	template<typename CompClass>
	class ComponentRef
	{
	public:
		ComponentRef()
			: handle(0), holder(nullptr)
		{}

		ComponentRef(ComponentHandle hdl, ComponentHolder* ho)
			: handle(hdl), holder(ho)
		{}

		const CompClass* operator -> () const {
			// printf("Calling CONST operator\n");
			return (const CompClass*)holder->getComponent(handle);
		}

		CompClass* operator -> () {
			// printf("Calling NORMAL operator\n");
			return static_cast<CompClass*>(holder->getComponent(handle));
		}

		// Some components need to be passed as parameters for other functions
		// The component handle should be passed.
		ComponentHandle getComponentHandle() const {
			return handle;
		}

	private:
		friend class BaseEntitySystem;
		ComponentHandle handle;
		ComponentHolder* holder;
	};


	/**
	 * Creates a Global Unique ID for the given component inside the application
	 */
	template<typename T>
	class ComponentGlobalID
	{
		public:
			static int ID()
			{
				static int x;
				return (int)&x;
			}
	};

	class BaseEntitySystem : public System
	{
		public:
			BaseEntitySystem()
				: System("Entity"){
				m_entities.emplace_back(0); // First entity is invalid.
			}

			/**
			* Creates a new Entity
			*/
			EntityHandle CreateEntity();

			/**
			* Destroy the entity
			* All components inside this entity will be destroyed too
			*
			* \param entity Entity to be destroyed
			*/
			void DestroyEntity(EntityHandle entity);

			template<typename CompClass>
			bool AddComponent(EntityHandle entity, ComponentType type, ComponentRef<CompClass>& ref)
			{
				if (!hasEntity(entity)) {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to attach component of type " << type << " to unknown entity: " << entity << "!";
					return false;
				}

				ComponentHolder* holder = m_dataHolder[type];
				ComponentHandle hdl = holder->CreateComponent(entity);
				if (hdl == 0) {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Entity " << entity << " already has a component of type: " << type << "!";
					return false;
				}

				ref.handle = hdl;
				ref.holder = holder;

				return true;
			}

			template<typename CompClass>
			bool RemoveComponent(const ComponentRef<CompClass>& ref) {
				if (!hasEntity(ref->getEntity())) {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to remove component of type " << type << " with handle: " << ref.handle << "!";
					return false;
				}

				ref.holder->ReleaseComponent(ref.handle);
				return true;
			}

			/**
			 * Get the component holder of given ComponentType
			 */
			inline ComponentHolder* getHolder(ComponentType type) {
				return m_dataHolder[type];
			}

			inline const ComponentHolder* getHolder(ComponentType type) const {
				return m_dataHolder[type];
			}

			// [Expensive] Avoid calling this multiple times!
			template<typename CompClass>
			ComponentType getComponentType() const
			{
				int id = ComponentGlobalID<CompClass>::ID();

				// TODO: Implement a better search?
				for (ComponentHolder* h : m_dataHolder)
				{
					if (h->globalID == id){
						return h->componentType;
					}
				}

				return ~0;
			}

			/**
			* Get a ComponentRef for given entity
			*
			* \param entity Entity to get the component from
			* @return Returns true when a component reference was found (and returned in ref).
			*/
			template<typename CompClass>
			bool getComponentRef(EntityHandle entity, ComponentType type, ComponentRef<CompClass>& ref) const
			{
				ComponentHolder* holder = m_dataHolder[type];
				ComponentHandle hdl = holder->getComponentHandleFor(entity);
				if (hdl != 0)
				{
					ref.holder = holder;
					ref.handle = hdl;
					return true;
				}

				return false;
			}

			/**
			* Get a ComponentRef for given entity
			*
			* \param entity Entity to get the component from
			* @return Returns true when a component reference was found (and returned in ref).
			*/
			template<typename ComponentIDProvider, typename CompClass>
			bool getComponentRef(EntityHandle entity, ComponentRef<CompClass>& ref) const
			{
				return getComponentRef<CompClass>(entity, ComponentIDProvider::ID<CompClass>(), ref);
			}

			/**
			* [Expensive] Avoid calling multiple times!
			* Get a ComponentRef for given entity
			* Prefer one of the two variations above if the ComponentType ID is known
			*
			* \param entity Entity to get the component from
			* @return Returns true when a component reference was found (and returned in ref).
			*/
			template<typename CompClass>
			bool getComponentRef_ex(EntityHandle entity, ComponentRef<CompClass>& ref) const
			{
				// Verify if component type is valid
				ComponentType type = getComponentType<CompClass>();
				if (type == ~0) {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Unregistered type: " << type;
					return false;
				}

				return getComponentRef(entity, type, ref);
			}

		protected:
			/* Register holder with given ids
			 * \param id is the sequential ID inside the BaseEntitySystem
			 * \param globalID is an unique application global component class identificator
			 *			this is used to discover the ComponentHolder inside this class 
			 *			based on the ComponentClass only.
			 *	See: getComponentRef
			 */
			bool RegisterHolder(ComponentHolder* holder, int id, int globalID);

			// TODO: Make this a DebugAssert?
			inline bool hasEntity(EntityHandle entity) const {
				return m_entities.size() > entity && m_entities[entity] == entity;
			}

			/**
			 * Should be called once the Update() is finished, after all Processors have finished execution
			 * for this frame.
			 * Releases all memory used by destroyed components/entities
			 */
			void FrameFinished();

			std::vector< EntityHandle > m_entities;
			std::vector< EntityHandle > m_freeEntities;
			std::vector< EntityHandle > m_toBeDestroyed;

			std::vector<ComponentHolder*> m_dataHolder;
	};

}