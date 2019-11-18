#pragma once

#include "BitEngine/Core/ECS/RenderableMeshComponent.h"

#include "BitEngine/Core/ECS/ComponentProcessor.h"
#include "BitEngine/Core/ECS/EntitySystem.h"


namespace BitEngine
{

	class RenderableMeshProcessor : public ComponentProcessor
	{
	public:
		RenderableMeshProcessor(EntitySystem* es);

		bool Init() override;
		void Stop() override;

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
		{}
	};


}