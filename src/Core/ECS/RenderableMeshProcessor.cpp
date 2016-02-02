#include "Core/ECS/RenderableMeshProcessor.h"

namespace BitEngine{

	COMPONENT_CLASS_IMPLEMENTATION(RenderableMeshComponent);

	RenderableMeshProcessor::RenderableMeshProcessor()
	{
	}
	
	bool RenderableMeshProcessor::Init(BaseEntitySystem* es){
		return true;
	}

	void RenderableMeshProcessor::Stop() {

	}

	ComponentHandle RenderableMeshProcessor::AllocComponent()
	{
		return components.newComponent();
	}

	void RenderableMeshProcessor::DeallocComponent(ComponentHandle component)
	{
		components.removeComponent(component);
	}

	Component* RenderableMeshProcessor::getComponent(ComponentHandle component)
	{
		return components.getComponent(component);
	}

	const std::vector<ComponentHandle>& RenderableMeshProcessor::getComponents() const
	{
		return components.getValidComponents();
	}

}