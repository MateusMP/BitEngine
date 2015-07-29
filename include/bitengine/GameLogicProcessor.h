#pragma once

#include "LogicScript.h"
#include "ComponentProcessor.h"

namespace BitEngine{

	class GameLogicProcessor : public ComponentHolderProcessor
	{
	public:
		GameLogicProcessor(EntitySystem *sys);
		~GameLogicProcessor();

		bool Init() override;
		void FrameStart() override;
		void FrameMiddle() override;
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		ComponentCollection<GameLogicComponent> components;

		std::vector<ComponentHandle> m_notInitialized;

		std::vector<GameLogic*> m_onFrameStart;
		std::vector<GameLogic*> m_onFrameMiddle;
		std::vector<GameLogic*> m_onFrameEnd;

		EntitySystem* m_e_sys;
	};

}