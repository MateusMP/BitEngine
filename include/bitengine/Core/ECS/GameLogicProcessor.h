#pragma once

#include "Core/ECS/LogicScript.h"
#include "Core/ECS/ComponentProcessor.h"

namespace BitEngine{

	class GameLogicProcessor : public ComponentProcessor
	{
	public:
		GameLogicProcessor();
		~GameLogicProcessor();

		/// Processor
		bool Init() override;
		void Stop() override;

		void FrameStart();
		void FrameMiddle();
		void FrameEnd();

	private:
		void onGameLogicComponentCreated(const BaseMessage& msg_);
		void onGameLogicComponentDestroyed(const BaseMessage& msg_);

		/// Processor
		void removeFrom(GameLogic* l, std::vector<GameLogic*>& vec);

	private:
		/// Processor
		ComponentType GameLogicComponentType;
		ComponentHolder<GameLogicComponent>* gameLogicHolder;

		std::vector<ComponentHandle> m_notInitialized;

		std::vector<GameLogic*> m_onFrameStart;
		std::vector<GameLogic*> m_onFrameMiddle;
		std::vector<GameLogic*> m_onFrameEnd;
	};

}