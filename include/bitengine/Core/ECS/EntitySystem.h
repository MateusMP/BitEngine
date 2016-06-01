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

		void InitComponentProcessor(ComponentProcessor* cp)
		{
			cp->m_es = this;
			cp->setMessenger(getMessenger());
			cp->Init();
		}

		// Register a component processor
		// pipeline: Which pipeline the processor should be inserted into
		//			All processors inside a pipeline are executed sequentially.
		//			Order of execution of processors from different pipelines are not guaranteed
		// cp: The component processor
		// func: The function that should be called on the given processor
		// Return true if the processor was added
		bool RegisterComponentProcessor(int pipeline, ComponentProcessor* cp, ComponentProcessor::processFunc func)
		{
			if (pipeline >= 4) { // limit pipelines
				return false;
			}
			process_order[pipeline].emplace_back(cp, func);

			// Verify if it was already added
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
			//std::thread run1(std::bind(&EntitySystem::runUpdate, this, std::placeholders::_1), 0);
			//std::thread run2(std::bind(&EntitySystem::runUpdate, this, std::placeholders::_1), 1);
			//std::thread run3(std::bind(&EntitySystem::runUpdate, this, std::placeholders::_1), 2);
			//std::thread run4(std::bind(&EntitySystem::runUpdate, this, std::placeholders::_1), 3);

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
			//run1.join();
			//run2.join();
			//run3.join();
			//run4.join();

			frameFinished();
		}

		template<typename CompClass>
		bool RegisterComponent(ComponentHolder<CompClass>* holder = nullptr, bool autoRegisterMsgType = true)
		{
			static_assert(!std::is_pod<CompClass>::value, "Components should be POD!");

			// Register message types
			if (autoRegisterMsgType)
			{
				MsgComponentCreated<CompClass>::MessageType();
				MsgComponentDestroyed<CompClass>::MessageType();
			}

			if (holder == nullptr)
			{
				LOG(EngineLog, BE_LOG_WARNING) << "Using generic ComponentHolder for " << typeid(CompClass).name();
				holder = new ComponentHolder<CompClass>();
			}

			if (!registerComponentHolder(CompClass::getComponentType(), holder))
			{
				delete holder;
				return false;
			}

			return true;
		}

		template<typename CompClass>
		ComponentHolder<CompClass>* getHolder()
		{
			return static_cast<ComponentHolder<CompClass>*>(BaseEntitySystem::getHolder(CompClass::getComponentType()));
		}

		template<typename CompClass>
		const ComponentHolder<CompClass>* getHolder() const
		{
			return static_cast<const ComponentHolder<CompClass>*>(BaseEntitySystem::getHolder(CompClass::getComponentType()));
		}

		// Add component
		template<typename CompClass, typename ... Args>
		ComponentRef<CompClass> AddComponent(EntityHandle entity, Args... args)
		{
			CompClass *comp = nullptr;
			ComponentHandle compID = BE_NO_COMPONENT_HANDLE;
			ComponentType type = CompClass::getComponentType();

			if (BaseEntitySystem::addComponent(entity, type))
			{
				ComponentHolder<CompClass>* holder = getHolder<CompClass>();
				compID = holder->createComponent(entity, comp, args...);

				getMessenger()->SendMessage(MsgComponentCreated<CompClass>(entity, type, compID));
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
		ComponentRef<CompClass> getComponentRef(EntityHandle entity)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			ComponentHandle compID = holder->getComponentForEntity(entity);
			CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

			return ComponentRef<CompClass>(entity, compID, this, comp);
		}


		template <typename T> struct identity{ typedef T type; };

		template<typename Base, typename ... ContainComps>
		void forEach(typename identity<std::function<void(EntityHandle, Base&, ContainComps&...)>>::type f)
		{
			// TODO: otimize calls of getComponentRefE with this:
			// std::tuple<ComponentHolder<ContainComps>*... > holders = std::make_tuple{ getHolder<ContainComps>()... };

			ComponentType types[] = { ContainComps::getComponentType()... };

            ComponentHolder<Base>* holder = getHolder<Base>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();

			BitMask componentMask{0};
			for (ComponentType t : types){
                componentMask.b64 |= 1ull << t;
			}

			// loop through base components searching for matching pairs on Args types
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				const EntityHandle entity = allComponents[compID];

				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
				    // Test to see if this entity has all needed components
                    if ( (m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64 )
                    {
                        Base* comp = holder->getComponent(compID);
                        f(entity, *comp, getComponentRefE<ContainComps>(entity)...); // TODO: avoid getHolder<>() every time
                    }
				}
				else
                {
					++curFree;
					++validComponents;
				}
			}
		}

		template<typename Caller, typename Base, typename ... ContainComps>
		void forEach(Caller& caller, typename identity<std::function<void(Caller&, EntityHandle, Base&, ContainComps&...)>>::type f)
		{
			ComponentType types[] = { ContainComps::getComponentType()... };

			ComponentHolder<Base>* holder = getHolder<Base>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();

			BitMask componentMask{0};
			for (ComponentType t : types){
                componentMask.b64 |= 1ull << t;
			}

			// loop through base components searching for matching pairs on Args types
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				const EntityHandle entity = allComponents[compID];

				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
					// Test to see if this entity has all needed components
					if ( (m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64 )
					{
						Base* comp = holder->getComponent(compID);

						f(caller, entity, *comp, getComponentRefE<ContainComps>(entity)...); // TODO: avoid getHolder<>() every time
					}
				}
				else
				{
					++curFree;
					++validComponents;
				}
			}
		}


		/*template< typename T>
		struct ComponentRefExtractor : public std::false_type {
			static ComponentRef<T> getRef(EntitySystem& es, EntityHandle entity) {
			}
		};

		template< typename CompClass>
		struct ComponentRefExtractor< Component<CompClass> >
		{
            static CompClass& getRef(EntitySystem& es, EntityHandle entity)
            {
                ComponentHolder<CompClass>* holder = es.getHolder<CompClass>();
                LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

                ComponentHandle compID = holder->getComponentForEntity(entity);
                return *static_cast<CompClass*>(holder->getComponent(compID));
            }
		};

		template< typename CompClass>
		struct ComponentRefExtractor< ComponentRef<CompClass> >
		{
            static ComponentRef<CompClass> getRef(EntitySystem& es, EntityHandle entity)
            {
                ComponentHolder<CompClass>* holder = es.getHolder<CompClass>();
                LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

                ComponentHandle compID = holder->getComponentForEntity(entity);
                CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

                return ComponentRef<CompClass>(entity, compID, &es, comp);
            }
		};

		template< typename T>
		struct ComponentTypeExtrator : public std::false_type {

		};

		template<typename X>
		struct ComponentTypeExtrator< Component<X> >
		{
			constexpr static ComponentType type() {
				return X::getComponentType();
			}
		};

		template<typename X>
		struct ComponentTypeExtrator< ComponentRef<X> >
		{
			constexpr static ComponentType type() {
				return X::getComponentType();
			}
		};


		template<typename Caller, typename Base, typename ... ContainComps>
		void forEachGeneric(Caller& caller, typename identity<std::function<void(Caller&, EntityHandle, Base&, ContainComps&&...)>>::type f)
		{
			ComponentType types[] = { ComponentTypeExtrator<ContainComps>::type()... };

			ComponentHolder<Base>* holder = getHolder<Base>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();

			BitMask componentMask{ 0 };
			for (ComponentType t : types) {
				componentMask.b64 |= 1ull << t;
			}

			// loop through base components searching for matching pairs on Args types
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				const EntityHandle entity = allComponents[compID];

				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
					// Test to see if this entity has all needed components
					if ((m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64)
					{
						Base* comp = holder->getComponent(compID);
						f(caller, entity, *comp, std::forward<ContainComps>(ComponentRefExtractor<ContainComps>::getRef(*this, entity))...); // TODO: avoid getHolder<>() every time
					}
				}
				else
				{
					++curFree;
					++validComponents;
				}
			}
		}*/


		template<typename Caller, typename Base, typename ... ContainComps>
		void forEachRef(Caller& caller, typename identity<std::function<void(Caller&, EntityHandle, Base&, ComponentRef<ContainComps>&&...)>>::type f)
		{
			ComponentType types[] = { ContainComps::getComponentType()... };

			ComponentHolder<Base>* holder = getHolder<Base>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);
			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			const auto& allComponents = holder->getAllComponents();
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();

			BitMask componentMask{0};
			for (ComponentType t : types){
                componentMask.b64 |= 1ull << t;
			}

			// loop through base components searching for matching pairs on Args types
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				const EntityHandle entity = allComponents[compID];

				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
					// Test to see if this entity has all needed components
					if ( (m_objBitField->getObj(entity).b64 & componentMask.b64) == componentMask.b64 )
					{
						Base* comp = holder->getComponent(compID);
						f(caller, entity, *comp, getComponentRef<ContainComps>(entity)...); // TODO: avoid getHolder<>() every time
					}
				}
				else
				{
					++curFree;
					++validComponents;
				}
			}
		}

		template<typename CompClass>
		void forAll(typename identity<std::function<void(ComponentHandle, CompClass&)>>::type f)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
					CompClass* comp = holder->getComponent(compID);
					f(compID, *comp);
				}
				else {
					++curFree;
					++validComponents;
				}
			}
		}

		template<typename Caller, typename CompClass>
		void forAll(Caller& caller, typename identity<std::function<void(Caller&, ComponentHandle, CompClass&)>>::type f)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			const auto& freeIDs = holder->getFreeIDs(); // sorted ids
			uint32 curFree = 0;

			uint32 validComponents = holder->getNumValidComponents();
			for (uint32 compID = 1; compID <= validComponents; ++compID)
			{
				if ((curFree < freeIDs.size() && freeIDs[curFree] != compID) || curFree >= freeIDs.size())
				{
					CompClass* comp = holder->getComponent(compID);
					f(caller, compID, *comp);
				} else {
					++curFree;
					++validComponents;
				}
			}
		}

		template<typename CompClass>
		uint32 getNumberOfValidComponents() const
		{
			return getHolder<CompClass>()->getNumValidComponents();
		}

	private:
		void runUpdate(int pipeline)
		{
			for (uint32 i = 0; i < process_order[pipeline].size(); ++i)
			{
				process_order[pipeline][i].Run();
			}
		}


		template<typename CompClass>
		CompClass& getComponentRefE(EntityHandle entity)
		{
			ComponentHolder<CompClass>* holder = getHolder<CompClass>();
			LOGIFNULL(EngineLog, BE_LOG_ERROR, holder);

			ComponentHandle compID = holder->getComponentForEntity(entity);
			CompClass* comp = static_cast<CompClass*>(holder->getComponent(compID));

			return *comp;
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
