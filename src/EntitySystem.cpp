
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
		sortProcess();
		ordered = true;

		for (auto& it : process_order[0]){
			if (!it.second->Init()){
				return false;
			}
		}
	
		return true;
	}

	void EntitySystem::Update(){

		if (!ordered)
		{
			sortProcess();
			ordered = true;
		}
		
		for (auto& it : process_order[1]){
			it.second->FrameStart();
		}

		for (auto& it : process_order[2]){
			it.second->FrameMiddle();
		}

		for (auto& it : process_order[3]){
			it.second->FrameEnd();
		}
		
	}

	void EntitySystem::Shutdown(){
		for (auto& it : process_order[0]){
			delete it.second;
		}
	}

	ResourceSystem* EntitySystem::getResourceSystem()
	{
		return m_resources;
	}

	void EntitySystem::sortProcess()
	{
		for (int i = 0; i < 3; ++i)
		{
			std::sort(process_order[i].begin(), process_order[i].end(),
				[](std::pair<int, ComponentProcessor*>& a, std::pair<int, ComponentProcessor*>& b)
			{
				return a.first < b.first;
			});
		}
	}
}