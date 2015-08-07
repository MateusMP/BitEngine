#pragma once

#include <unordered_map>

#include "Sprite2DComponent.h"
#include "Camera2DProcessor.h"

namespace BitEngine{

class Sprite2DProcessor : public ComponentHolderProcessor
{
	public:
		Sprite2DProcessor();
		~Sprite2DProcessor();
		
	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override{}

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		/*
		* \param screen Screen expected to be a 2D box with bottom left coordinates to top right coordinates
		* \param t Transform2D to be used on calculation
		* \param s Sprite2D to be used on calculation
		*/
		bool insideScreen(const glm::vec4& screen, const Transform2DComponent* t, const Sprite2DComponent* s);

	private:
		ComponentCollection<Sprite2DComponent> components;
};

}