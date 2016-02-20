#pragma once

#include <array>
#include <map>
#include <unordered_map>
#include <functional>

#include <algorithm>

#include "Common/TypeDefinition.h"
#include "Core/Logger.h"

#include "Core/ECS/BaseEntitySystem.h"
#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine{

/**
* Entity Handle and Component Handle are fixed and won't change at anytime after creation.
* Type safe interface
*/
class EntitySystem : public BaseEntitySystem
{
	public:
		EntitySystem()
		{
		}

		bool Init()
		{
			BaseEntitySystem::Init();

			for (ComponentProcessor* p : m_processors)
			{
				p->m_es = this;
				p->setMessenger(getMessenger());
				p->Init();
			}

			return true;
		}

		void Shutdown()
		{
			for (ComponentProcessor* p : m_processors)
			{
				p->Stop();
			}

			BaseEntitySystem::shutdown();
		}

		// Register

		bool RegisterComponentProcessor(int pipeline, ComponentProcessor* cp, ComponentProcessor::processFunc func)
		{
			if (pipeline >= 4) { // limit pipelines
				return false;
			}
			process_order[pipeline].emplace_back(cp, func);

			bool inside = false;
			for (ComponentProcessor* p : m_processors)
			{
				if (p == cp)
				{
					inside = true;
					break;
				}
			}

			if (!inside)
			{
				m_processors.emplace_back(cp);
			}

			return true;
		}


		void Update()
		{
			int finish = 0;
			size_t lasts[4] = { 0,0,0,0 };

			// TODO: Use threads
			while (finish != (0xf) )
			{
				for (int i = 0; i < 4; ++i)
				{
					if (!(finish & (1 << i)))
					{
						if (lasts[i] < process_order[i].size())
						{
							process_order[i][lasts[i]++].Run();
						}
						else
						{
							finish |= (1 << i);
						}
					}
				}

			}

			FrameFinished();
		}

		template<typename CompClass>
		bool RegisterComponent()
		{
			ComponentHolder<CompClass>* ptr = new ComponentHolder<CompClass>();
			if (!registerComponentHolder(CompClass::getComponentType(), ptr)) {
				delete ptr;
				return false;
			}

			return true;
		}

		template<typename CompClass>
		ComponentHolder<CompClass>* getHolder()
		{
			return static_cast<ComponentHolder<CompClass>*>(BaseEntitySystem::getHolder(CompClass::getComponentType()));
		}

		// Add component
		template<typename CompClass, typename ... Args>
		ComponentRef<CompClass> AddComponent(EntityHandle entity, Args... args)
		{
			CompClass *comp = nullptr;
			ComponentHolder<CompClass>* holder;
			ComponentHandle compID = NO_COMPONENT_HANDLE;
			ComponentType type = CompClass::getComponentType();

			if (BaseEntitySystem::addComponent(entity, type))
			{
				holder = getHolder<CompClass>();
				compID = holder->createComponent(comp, args...);
			}

			return ComponentRef<CompClass>(entity, compID, this, comp);
		}

		template<typename CompClass>
		bool RemoveComponent(EntityHandle entity, const ComponentRef<CompClass>& ref)
		{
			return BaseEntitySystem::removeComponent(entity, CompClass::getComponentType(), ref.m_componentID);
		}

		/**
		* Get a ComponentRef for given entity
		*
		* \param entity Entity to get the component from
		* @return Returns true when a component reference was found (and returned in ref).
		*/
		template<typename CompClass>
		ComponentRef<CompClass> getComponentRef(EntityHandle entity) const
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			ComponentHandle compID = holder->getComponentForEntity(entity);
			CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

			return ComponentRef<CompClass>(entity, compID, this, comp);
		}


		template <typename T> struct identity{ typedef T type; };
		template<typename Base, typename ... Args>
		void forEach(typename identity<std::function<void(EntityHandle, Base&, Args&...)>>::type f)
		{
			ComponentType types[] = { Args::getComponentType()... };

            ComponentHolder<Base>* holder = getHolder<Base>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();

			// loop through base components searching for matching pairs on Args types
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				const EntityHandle entity = allComponents[compID];

				if (curFree < freeIDs.size() && freeIDs[curFree] != compID)
				{
				    // Test to see if this entity has all needed components
                    if (testBitfieldArray<sizeof...(Args)>(entity, types))
                    {
                        Base* comp = holder->getComponent(compID);
                        f(entity, *comp, getComponentRefE<Args>(entity)...); // TODO: avoid getHolder<>() every time
                    }
				}
				else
                {
					++curFree;
					++validComponents;
				}
			}
		}

		template<typename CompClass, typename ...Others>
		void forAll(typename identity<std::function<void(ComponentHandle, CompClass&, Others...)>>::type f, Others ... others)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			//const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;


			uint32 validComponents = holder->getNumValidComponents();
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				// const EntityHandle entity = allComponents[compID];

				if (curFree < freeIDs.size() && freeIDs[curFree] != compID)
				{
					CompClass* comp = holder->getComponent(compID);
					f(compID, *comp, others...);
				} else {
					++curFree;
					++validComponents;
				}
			}
		}


	private:
		template<typename CompClass>
		CompClass& getComponentRefE(EntityHandle entity)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			ComponentHandle compID = holder->getComponentForEntity(entity);
			CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

			return *comp;
		}

		template<int n>
		bool testBitfieldArray(EntityHandle e, ComponentType *types)
		{
			for (int i = 0; i < n; ++i) {
				if (!m_objBitField->test(e, types[i])) {
					return false;
				}
			}
			return true;
		}

		struct PipelineProcess
		{
			PipelineProcess(ComponentProcessor* c, ComponentProcessor::processFunc f) :
				cs(c), func(f)
			{}

			ComponentProcessor* cs;
			ComponentProcessor::processFunc func;

			void Run()
			{
				(cs->*func)();
			}
		};

		std::array< std::vector<PipelineProcess>, 4 > process_order;
		std::vector<ComponentProcessor* > m_processors; // Unique component processors
};

}
