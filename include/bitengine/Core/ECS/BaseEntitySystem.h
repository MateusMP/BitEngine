#pragma once

#include <vector>

#include "Common/BitFieldVector.h"
#include "Core/System.h"
#include "Core/ECS/Component.h"
#include "Core/ECS/ComponentProcessor.h"
#include "Core/Logger.h"

namespace BitEngine {

	class BaseEntitySystem : public System
	{
		public:
			template<typename CompClass>
			inline static GlobalComponentID getGlobalComponentID()
			{
				return CompClass::getGlobalComponentID();
			}

			BaseEntitySystem()
				: System("Entity")
			{
				m_objBitField = nullptr;
				m_entities.emplace_back(0); // First entity is invalid.
			}

			bool Init() override
			{
				m_objBitField = new ObjBitField(static_cast<uint16>(m_holders.size()));

				return true;
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

			bool addComponent(EntityHandle entity, ComponentType type)
			{
				if (!hasEntity(entity))
                {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to attach component of type " << type << " to unknown entity: " << entity << "!";
					return false;
				}

				m_objBitField->set(entity, type);

				return true;
			}

			bool removeComponent(EntityHandle entity, ComponentType type, ComponentHandle handle)
			{
				if (!hasEntity(entity))
                {
					LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to remove component of type " << type << " with handle: " << handle << " from entity " << entity << "!";
					return false;
				}

				m_holders[type]->releaseComponentID(handle);

				return true;
			}

			bool isComponentOfTypeValid(ComponentType type) const {
				return m_holders.find(type) != m_holders.end();
			}

			/**
			* Get the component holder of given ComponentType
			*/
			inline BaseComponentHolder* getHolder(ComponentType type) {
				return m_holders[type];
			}

			inline const BaseComponentHolder* getHolder(ComponentType type) const
			{
				auto it = m_holders.find(type);
				if (it != m_holders.end())
					return it->second;
				return nullptr;
			}

		protected:
			bool hasComponent(EntityHandle entity, ComponentType type)
			{
				return m_objBitField->test(entity, type);
			}

			void shutdown()
			{
				for (auto& h : m_holders)
				{
					delete h.second;
				}
			}

			bool registerComponentHolder(ComponentType type, BaseComponentHolder* holder)
			{
				auto it = m_holders.find(type);
				if (it != m_holders.end()) {
					return false;
				}

				m_holders[type] = holder;
				return true;
			}

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
			ObjBitField *m_objBitField;

		private:
			std::unordered_map< ComponentType, BaseComponentHolder* > m_holders;
	};

}
