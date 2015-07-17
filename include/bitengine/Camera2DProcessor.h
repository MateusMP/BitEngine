#pragma once

#include "Camera2DComponent.h"
#include "ComponentProcess.h"

namespace BitEngine{

class Camera2DProcessor : public ComponentHolderProcess<Camera2DComponent>
{
	public:
		Camera2DProcessor();

		Camera2DComponent* getActiveCamera();

		std::vector<Camera2DComponent*>& getComponents() override;

	private:
		bool Init() override;
		void Process() override;

		ComponentHandle CreateComponent() override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		
	private:
		ComponentCollection<Camera2DComponent> components;

		Camera2DComponent* activeCamera;
};

}