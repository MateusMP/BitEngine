#pragma once

#include <unordered_set>

#include "BitEngine/Core/ECS/Camera3DComponent.h"
#include "BitEngine/Core/ECS/ComponentProcessor.h"

#include "BitEngine/Core/ECS/Transform3DProcessor.h"
#include "BitEngine/Core/ECS/EntitySystem.h"


namespace BitEngine{

	class BE_API Camera3DProcessor : public ComponentProcessor
	{
	public:
		Camera3DProcessor(EntitySystem* es, Transform3DProcessor* t3dp);

		/// Processor
		void Process();

	private:

		//
		static void recalculateViewMatrix(Camera3DComponent& c, const glm::mat4& t);

	private:
		// Processor
		struct Entry {
			Entry(EntityHandle ent, ComponentHandle cam, ComponentHandle transf)
				: entity(ent), camera3d(cam), transform3d(transf) {}
			EntityHandle entity;
			ComponentHandle camera3d;
			ComponentHandle transform3d;
		};

		// Hold the ComponentType for all components this processor cares
		ComponentType Transform3DType;
		ComponentType Camera3DType;

		// Hold the ComponentHolders for all ComponentTypes cared by this processor
		ComponentHolder<Camera3DComponent>* holderCamera;
		ComponentHolder<Transform3DComponent>* holderTransform;
		Transform3DProcessor* transform3DProcessor;
	};

}
