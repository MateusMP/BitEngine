#pragma once

#include "Camera2DComponent.h"
#include "ComponentProcessor.h"

#include "Transform2DProcessor.h"
#include "EntitySystem.h"

namespace BitEngine{

class Camera2DProcessor : public ComponentHolderProcessor
{
	public:
		Camera2DProcessor(EntitySystem* es);

		Component* getComponent(ComponentHandle component) override;

	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

		//
		void recalculateMatrix(Camera2DComponent* c, const Transform2DComponent* t);
		
	private:
		EntitySystem* m_entitySys;

		ComponentCollection<Camera2DComponent> components;
};

}