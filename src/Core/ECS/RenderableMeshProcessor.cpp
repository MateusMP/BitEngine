#include "Core/ECS/RenderableMeshProcessor.h"

namespace BitEngine{

	RenderableMeshProcessor::RenderableMeshProcessor(Messenger* m)
		: ComponentProcessor(m)
	{
	}
	
	bool RenderableMeshProcessor::Init(){
		return true;
	}

	void RenderableMeshProcessor::Stop() {

	}

}