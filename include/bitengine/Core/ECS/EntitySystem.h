#pragma once

#include <array>
#include <map>
#include <unordered_map>

#include <algorithm>

#include "Common/TypeDefinition.h"
#include "Core/Logger.h"

#include "Core/ECS/BaseEntitySystem.h"
#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine{

template <typename T, typename... Ts> struct get_index;

template <typename T, typename... Ts>
struct get_index<T, T, Ts...> : std::integral_constant<std::size_t, 0> {};

template <typename T, typename Tail, typename... Ts>
struct get_index<T, Tail, Ts...> :
	std::integral_constant<std::size_t, 1 + get_index<T, Ts...>::value> {};

template<typename ... T>
class ComponentIDProvider
{
public:
	template<typename A>
	static constexpr ComponentType ID() {
		return get_index<A, T...>();
	}

	static constexpr int TotalComponents() {
		return sizeof...(T);
	}
};


/**
* Entity Handle and Component Handle are fixed and won't change at anytime after creation.
*/
template<typename ComponentIDProvider>
class EntitySystem : public BaseEntitySystem
{
	public:
		EntitySystem()
		{
			m_dataHolder.resize(ComponentIDProvider::TotalComponents(), nullptr);
		}

		bool Init()
		{
			for (ComponentProcessor* p : m_processors)
			{
				p->setMessenger(getMessenger());
				p->Init(this);
			}

			return true;
		}

		void Shutdown()
		{
			for (ComponentProcessor* p : m_processors)
			{
				p->Stop();
			}

			for (ComponentHolder* h : m_dataHolder)
			{
				bool del = true;
				for (auto it = m_processors.begin(); it != m_processors.end(); ++it)
				{
					// Remove duplicates, because some Processors are Holders too
					if ((void*)(*it) == (void*)h)
					{
						delete h;
						del = false;
						it = m_processors.erase(it);
						break;
					}
				}

				if (del) 
				{
					delete h;
				}
			}
		}

		// Register

		template<typename CompClass>
		bool RegisterComponentHolder(ComponentHolder* ch)
		{
			const ComponentType type = ComponentIDProvider::template ID<CompClass>();
			GlobalComponentID globalID = getGlobalComponentID<CompClass>();

			if (!RegisterHolder(ch, type, globalID))
			{
				LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to register two holders for the component type: " << type;
				return false;
			}

			return true;
		}

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

		bool isComponentOfTypeValid(ComponentType type) const {
			return m_dataHolder.size() > type && m_dataHolder[type] != nullptr;
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

		// Add

		template<typename CompClass>
		bool AddComponent(EntityHandle entity, ComponentRef<CompClass>& ref)
		{
			return BaseEntitySystem::AddComponent(entity, ComponentIDProvider::template ID<CompClass>(), ref);
		}

		template<typename CompClass>
		bool AddComponent(EntityHandle entity)
		{
			ComponentRef<CompClass> ref;
			return BaseEntitySystem::AddComponent(entity, ComponentIDProvider::template ID<CompClass>(), ref);
		}

		template<typename CompClass>
		bool RemoveComponent(EntityHandle entity, const ComponentRef<CompClass>& ref)
		{
			return BaseEntitySystem::RemoveComponent(entity, ComponentIDProvider::template ID<CompClass>(), ref);
		}

	private:

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
