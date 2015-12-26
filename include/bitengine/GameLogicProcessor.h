#pragma once

#include "LogicScript.h"
#include "ComponentProcessor.h"

namespace BitEngine{

	class GameLogicProcessor : public ComponentProcessor, public ComponentHolder
	{
	public:
		GameLogicProcessor();
		~GameLogicProcessor();

		/// Processor
		bool Init(BaseEntitySystem* es) override;
		void Stop() override;

		void FrameStart();
		void FrameMiddle();
		void FrameEnd();

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override;
		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override;

	private:

		/// Holder
		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		const std::vector<ComponentHandle>& getComponents() const override;

		/// Processor
		void removeFrom(GameLogic* l, std::vector<GameLogic*>& vec);

	private:
		/// Holder
		ComponentCollection<GameLogicComponent> components;

		/// Processor
		BaseEntitySystem* e_sys;

		std::vector<ComponentHandle> m_notInitialized;

		std::vector<GameLogic*> m_onFrameStart;
		std::vector<GameLogic*> m_onFrameMiddle;
		std::vector<GameLogic*> m_onFrameEnd;

		ComponentType GameLogicComponentType;
	};

}