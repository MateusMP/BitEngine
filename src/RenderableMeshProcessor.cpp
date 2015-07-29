#include "RenderableMeshProcessor.h"

namespace BitEngine{

	RenderableMeshProcessor::RenderableMeshProcessor()
	{
	}
	
	bool RenderableMeshProcessor::Init(){
		return true;
	}

	ComponentHandle RenderableMeshProcessor::CreateComponent(EntityHandle entity)
	{
		return components.newComponent();
	}

	void RenderableMeshProcessor::DestroyComponent(ComponentHandle component)
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