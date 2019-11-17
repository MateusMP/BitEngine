#include "bitengine/Core/ECS/RenderableMeshProcessor.h"

namespace BitEngine{

	RenderableMeshProcessor::RenderableMeshProcessor(EntitySystem* es)
		: ComponentProcessor(es)
	{
	}
	
	bool RenderableMeshProcessor::Init(){
		return true;
	}

	void RenderableMeshProcessor::Stop() {

	}

}