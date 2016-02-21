#include "Core/ECS/GameLogicProcessor.h"

namespace BitEngine{

	GameLogicProcessor::GameLogicProcessor()
	{
	}

	GameLogicProcessor::~GameLogicProcessor()
	{

	}

	bool GameLogicProcessor::Init()
	{
		gameLogicHolder = getES()->getHolder<GameLogicComponent>();
		getMessenger()->RegisterListener<MsgComponentCreated<GameLogicComponent> >(this, BE_MESSAGE_HANDLER(GameLogicProcessor::onGameLogicComponentCreated));
		getMessenger()->RegisterListener<MsgComponentDestroyed<GameLogicComponent> >(this, BE_MESSAGE_HANDLER(GameLogicProcessor::onGameLogicComponentDestroyed));

		return true;
	}

	void GameLogicProcessor::Stop()
	{
		getES()->forAll<GameLogicComponent>(
			[](ComponentHandle handle, GameLogicComponent& l)
			{
				auto& pieces = l.m_gamelogics;
				for (GameLogic* logic : pieces) {
					logic->End();
				}
			}
		);
	}

	void GameLogicProcessor::FrameStart()
	{
		// LOG(EngineLog, BE_LOG_VERBOSE) << "GameLogicProcessor::FrameStart\n";

		// Initialize all components not yet initialized
		for (ComponentHandle hdl : m_notInitialized)
		{
			auto& pieces = gameLogicHolder->getComponent(hdl)->m_gamelogics;
			for (size_t i = 0; i < pieces.size(); ++i)
			{
				GameLogic* logic = pieces[i];
				logic->setMessenger(getMessenger());
				GameLogic::RunEvents ev = logic->getRunEvents();
				if (ev & GameLogic::RunEvents::EFrameStart){
					m_onFrameStart.emplace_back(logic);
				}
				if (ev & GameLogic::RunEvents::EFrameMiddle){
					m_onFrameMiddle.emplace_back(logic);
				}
				if (ev & GameLogic::RunEvents::EFrameEnd){
					m_onFrameEnd.emplace_back(logic);
				}

				if (!logic->Init()){
					LOG(EngineLog, BE_LOG_ERROR) << "GameLogicProcessor: GameLogicComponent " << hdl << " logic " << i << " failed to Init! ";
				}
			}
		}
		m_notInitialized.clear();

		// Run FrameStart on all components
		for (GameLogic* l : m_onFrameStart){
			l->FrameStart();
		}
	}

	void GameLogicProcessor::FrameMiddle()
	{
		// LOG(EngineLog, BE_LOG_VERBOSE) << "GameLogicProcessor::FrameMiddle";

		for (GameLogic* l : m_onFrameMiddle){
			l->FrameMiddle();
		}
	}

	void GameLogicProcessor::FrameEnd()
	{
		// LOG(EngineLog, BE_LOG_VERBOSE) << "GameLogicProcessor::FrameEnd";

		for (GameLogic* l : m_onFrameEnd){
			l->FrameEnd();
		}
	}

	void GameLogicProcessor::onGameLogicComponentCreated(const BaseMessage& msg_)
	{
		const MsgComponentCreated<GameLogicComponent> msg = static_cast<const MsgComponentCreated<GameLogicComponent>&>(msg_);
		GameLogicComponent* glc =  gameLogicHolder->getComponent(msg.component);
		glc->e_sys = getES();
		glc->m_entity = msg.entity;

		m_notInitialized.emplace_back(msg.component);
	}

	void GameLogicProcessor::onGameLogicComponentDestroyed(const BaseMessage& msg_)
	{
		const MsgComponentDestroyed<GameLogicComponent> msg = static_cast<const MsgComponentDestroyed<GameLogicComponent>&>(msg_);
		GameLogicComponent* comp = gameLogicHolder->getComponent(msg.component);
		auto& pieces = comp->m_gamelogics;
		for (size_t i = 0; i < pieces.size(); ++i)
		{
			GameLogic* logic = pieces[i];
			GameLogic::RunEvents ev = logic->getRunEvents();
			if (ev & GameLogic::RunEvents::EFrameStart) {
				removeFrom(logic, m_onFrameStart);
			}

			if (ev & GameLogic::RunEvents::EFrameMiddle) {
				removeFrom(logic, m_onFrameMiddle);
			}

			if (ev & GameLogic::RunEvents::EFrameEnd) {
				removeFrom(logic, m_onFrameEnd);
			}

			logic->End();
		}
	}

	void GameLogicProcessor::removeFrom(GameLogic* l, std::vector<GameLogic*>& vec)
	{
		for (size_t i = 0; i < vec.size(); ++i)
		{
			if (vec[i] == l)
			{
				vec[i] = vec.back();
				vec.pop_back();
				return;
			}
		}
	}

}
