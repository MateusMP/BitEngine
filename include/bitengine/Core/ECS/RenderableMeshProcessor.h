#pragma once

#include "Core/ECS/RenderableMeshComponent.h"

#include "Core/ECS/ComponentProcessor.h"
#include "Core/ECS/EntitySystem.h"


namespace BitEngine
{

	class RenderableMeshProcessor : public ComponentProcessor
	{
	public:
		RenderableMeshProcessor();

		bool Init() override;
		void Stop() override;

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
	};


}