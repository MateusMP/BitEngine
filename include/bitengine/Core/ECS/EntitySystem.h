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
					if ((void*)(*it) == (void*)h)
					{
						delete h;
						del = false;
						m_processors.erase(it);
						break;
					}
				}

				if (del) {
					delete h;
				}
			}
		}

		// Register

		template<typename CompClass>
		bool RegisterComponentHolder(ComponentHolder* ch)
		{
			const ComponentType type = ComponentIDProvider::template ID<CompClass>();
			const int globalID = ComponentGlobalID::ID<CompClass>();

			if (!RegisterHolder(ch, type, globalID))
			{
				LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to register two holders for the component type: " << type;
				return false;
			}

			return true;
		}

		bool RegisterComponentProcessor(int pipeline, ComponentProcessor* cp, ComponentProcessor::processFunc func)
		{
			if (pipeline >= 4) {
				return false;
			}
			process_order[pipeline].emplace_back(cp, func);

			bool inside = false;
			for (ComponentProcessor* p : m_processors) {
				if (p == cp) {
					inside = true;
					break;
				}
			}
			if (!inside)
				m_processors.emplace_back(cp);

			return true;
		}

		bool isComponentOfTypeValid(ComponentType type) const {
			return m_dataHolder.size() > type && m_dataHolder[type] != nullptr;
		}

		void Update() {

			int finish = 0;
			size_t lasts[4] = { 0,0,0,0 };

			// TODO: Use threads
			while (finish != (0xf) )
			{
				for (int i = 0; i < 4; ++i)
				{
					if (!(finish & (1 << i))) {
						if (lasts[i] < process_order[i].size()) {
							process_order[i][lasts[i]++].Run();
						}
						else {
							finish |= (1 << i);
						}
					}
				}

			}

			FrameFinished();
		}



		// Get

		//// Search
		///**
		// * Find all components of type <Having>
		// * that matches with the entities of the components given in search
		// * \param search Components to look for owner entity required <having> components
		// * \param answer ComponentHandle found of Having for each search entry
		// * \param matchSearchIndices valid indices of matches found for search
		// * If an entity does not have at least one of the searching components, it won't be returned as part of the answer.
		// */
		//template<typename BaseSearchCompClass, typename... Having>
		//void findAllTuples(const std::vector<ComponentHandle> &search, std::vector<ComponentHandle>& answer, std::vector<uint32>& matchSearchIndices) const
		//{
		//	int nTypes = sizeof...(Having);
		//	ComponentType typeSearch = BaseSearchCompClass::getComponentType();
		//
		//	if (!isComponentOfTypeValid(typeSearch)){
		//		matchSearchIndices.clear(); // nothing found!
		//		LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to search for invalid type: " << typeSearch;
		//		return;
		//	}
		//
		//	// Find all entities IDs of search
		//	const DHP& dhp = m_dataHolder[typeSearch];
		//	int i;
		//	for (size_t k = 0; k < search.size(); ++k)
		//	{
		//		const ComponentHandle hdl = search[k];
		//		EntityHandle entity = dhp.getEntityFor(hdl);
		//		ComponentHandle componentsFound[] = { getComponentFromEntity<Having>(entity)... };
		//
		//		// printf("Entity: %d\n", entity);
		//
		//		for (i = 0; i < nTypes; ++i){
		//			// printf("%d ; ", componentsFound[i]);
		//			if (componentsFound[i] == 0) break;
		//		}
		//		// printf("\n");
		//		if (i == nTypes){
		//			// printf("Match found: ");
		//			for (i = 0; i < nTypes; ++i){
		//				answer.push_back(componentsFound[i]);
		//				// printf("%d, ", componentsFound[i]);
		//			}
		//			// printf("\n");
		//			matchSearchIndices.push_back(k);
		//		}
		//		else{
		//			// printf("jumping hdl: [%d]%d\n", k, hdl);
		//		}
		//
		//		/*ComponentHandle x = getComponentFromEntity<Having>(entity);
		//		if ((x != 0))
		//		{
		//			printf("X: %d\n", x);
		//			// answer.push_back(x)...;
		//		}*/
		//	}
		//
		//}
		//
		//// Get all components of BaseSearchCompClass with matching components Having
		//// The answer will have all components handle following the sequence of input
		//// Example: findAllTuplesOf<Camera2DComponent, Transform2DComponent>
		//// The answer format will be in pairs: [Camera2DComponentHandle Transform2DComponentHandle]
		//// answer: 1, 9, 2, 6 ... Where we have the following pairs: [1, 9][2, 6]
		//template<typename BaseSearchCompClass, typename... Having>
		//void findAllTuplesOf(std::vector<ComponentHandle>& answer) const
		//{
		//	int nTypes = sizeof...(Having);
		//	ComponentType typeSearch = BaseSearchCompClass::getComponentType();
		//
		//	if (!isComponentOfTypeValid(typeSearch)){
		//		LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to search for invalid type: " << typeSearch;
		//		return;
		//	}
		//
		//	const DHP& dhp = m_dataHolder[typeSearch];
		//	auto& components = dhp.processor->getComponents();
		//
		//	// Find all entities IDs of search
		//	int i;
		//	for (ComponentHandle hdl : components)
		//	{
		//		EntityHandle entity = dhp.getEntityFor(hdl);
		//		ComponentHandle componentsFound[] = { getComponentFromEntity<Having>(entity)... };
		//
		//		// printf("Entity: %d\n", entity);
		//
		//		for (i = 0; i < nTypes; ++i){
		//			// printf("%d ; ", componentsFound[i]);
		//			if (componentsFound[i] == 0) break;
		//		}
		//		// printf("\n");
		//		if (i == nTypes){
		//			// printf("Match found: ");
		//			answer.push_back(hdl);
		//			for (i = 0; i < nTypes; ++i){
		//				answer.push_back(componentsFound[i]);
		//				// printf("%d, ", componentsFound[i]);
		//			}
		//			// printf("\n");
		//		}
		//		else{
		//			// printf("jumping hdl: [%d]%d\n", k, hdl);
		//		}
		//
		//		/*ComponentHandle x = getComponentFromEntity<Having>(entity);
		//		if ((x != 0))
		//		{
		//		printf("X: %d\n", x);
		//		// answer.push_back(x)...;
		//		}*/
		//	}
		//}
		//
		//// Same as above but return reference pointers
		//template<typename BaseSearchCompClass, typename... Having>
		//void findAllTuplesOf(std::vector<Component*>& answer) const
		//{
		//	int nTypes = sizeof...(Having);
		//	ComponentType typeSearch = BaseSearchCompClass::getComponentType();
		//
		//	if (!isComponentOfTypeValid(typeSearch)){
		//		LOG(EngineLog, BE_LOG_WARNING) << "EntitySystem: Trying to search for invalid type: " << typeSearch;
		//		return;
		//	}
		//
		//	const DHP& dhp = m_dataHolder[typeSearch];
		//	auto& components = dhp.processor->getComponents();
		//
		//	// Find all entities IDs of search
		//	int i;
		//	for (ComponentHandle hdl : components)
		//	{
		//		EntityHandle entity = dhp.getEntityFor(hdl);
		//		Component* componentsFound[] = { getComponentRefFromEntity<Having>(entity)... };
		//
		//		// printf("Entity: %d\n", entity);
		//
		//		for (i = 0; i < nTypes; ++i){
		//			// printf("%d ; ", componentsFound[i]);
		//			if (componentsFound[i] == 0) break;
		//		}
		//
		//		// printf("\n");
		//		if (i == nTypes)
		//		{
		//			// printf("Match found: ");
		//			answer.push_back((BaseSearchCompClass*)dhp.processor->getComponent(hdl));
		//			for (i = 0; i < nTypes; ++i){
		//				answer.push_back(componentsFound[i]);
		//			}
		//		}
		//		else{
		//			// printf("jumping hdl: [%d]%d\n", k, hdl);
		//		}
		//
		//		/*ComponentHandle x = getComponentFromEntity<Having>(entity);
		//		if ((x != 0))
		//		{
		//		printf("X: %d\n", x);
		//		// answer.push_back(x)...;
		//		}*/
		//	}
		//
		//}


		// Add

		template<typename CompClass>
		bool AddComponent(EntityHandle entity, ComponentRef<CompClass>& ref)
		{
			return BaseEntitySystem::AddComponent(entity, ComponentIDProvider::template ID<CompClass>(), ref);
		}

		template<typename CompClass>
		bool RemoveComponent(EntityHandle entity, const ComponentRef<CompClass>& ref)
		{
			return BaseEntitySystem::RemoveComponent(entity, ComponentIDProvider::template ID<CompClass>(), ref);
		}

	private:
	/*
		template<typename CompClass>
		ComponentHandle getComponentFromEntity(EntityHandle entity) const
		{
			ComponentType type = CompClass::getComponentType();
			return m_dataHolder[type].getComponentFor(entity);
		}

		template<typename CompClass>
		CompClass* getComponentRefFromEntity(EntityHandle entity) const
		{
			ComponentType type = CompClass::getComponentType();
			return (CompClass*)m_dataHolder[type].getComponentRefFor(entity);
		}
		*/
		struct PipelineProcess {
			PipelineProcess(ComponentProcessor* c, ComponentProcessor::processFunc f) :
				cs(c), func(f)
			{}
			ComponentProcessor* cs;
			ComponentProcessor::processFunc func;

			void Run() {
				(cs->*func)();
			}
		};
		std::array< std::vector<PipelineProcess>, 4 > process_order;

		std::vector<ComponentProcessor* > m_processors; // Unique component processors
};

}
