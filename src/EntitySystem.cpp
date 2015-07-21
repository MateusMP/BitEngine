
#include "EntitySystem.h"

#include "Transform2DComponent.h"

namespace BitEngine{

	EntitySystem::EntitySystem(ResourceSystem *resource)
		: System("Entity"), nullDHP(nullptr, COMPONENT_TYPE_INVALID), ordered(false), m_resources(resource)
	{
		m_entities.emplace_back(0); // First entity is invalid.
	}

	bool EntitySystem::Init()
	{
		std::sort(process_order.begin(), process_order.end(),
			[](std::pair<int, ComponentProcessor*>& a, std::pair<int, ComponentProcessor*>& b)
		{
			return a.first < b.first;
		});
		ordered = true;

		for (auto& it : process_order){
			if (!it.second->Init()){
				return false;
			}
		}
		return true;
	}

	void EntitySystem::Update(){

		if (!ordered)
		{
			std::sort(process_order.begin(), process_order.end(),
				[](std::pair<int, ComponentProcessor*>& a, std::pair<int, ComponentProcessor*>& b)
			{
				return a.first < b.first;
			});
			ordered = true;
		}
		
		for (auto& it : process_order){
			it.second->Process();
		}
	}

	void EntitySystem::Shutdown(){
		for (auto& it : process_order){
			delete it.second;
		}
	}

	ResourceSystem* EntitySystem::getResourceSystem()
	{
		return m_resources;
	}
}