#pragma once

#include "bitengine/Core/ECS/RenderableMeshComponent.h"

#include "bitengine/Core/ECS/ComponentProcessor.h"
#include "bitengine/Core/ECS/EntitySystem.h"


namespace BitEngine
{

	class RenderableMeshProcessor : public ComponentProcessor
	{
	public:
		RenderableMeshProcessor(Messenger* m);

		bool Init() override;
		void Stop() override;

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
	};


}