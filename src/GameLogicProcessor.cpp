#include "GameLogicProcessor.h"

namespace BitEngine{

	GameLogicProcessor::GameLogicProcessor(EntitySystem* sys)
		: m_e_sys(sys)
	{
	}

	GameLogicProcessor::~GameLogicProcessor(){

		auto& C = components.getValidComponents();
		for (ComponentHandle component : C)
		{
			auto& pieces = components.getComponent(component)->m_gamelogics;
			for (GameLogic* logic : pieces){
				logic->End();
			}
		}
	}

	bool GameLogicProcessor::Init(){
		return true;
	}

	void GameLogicProcessor::FrameStart(){
		// LOGTO(Verbose) << "GameLogicProcessor::FrameStart\n" << endlog;

		for (ComponentHandle hdl : m_notInitialized)
		{
			auto& pieces = components.getComponent(hdl)->m_gamelogics;
			for (size_t i = 0; i < pieces.size(); ++i)
			{
				GameLogic* logic = pieces[i];
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
					LOGTO(Error) << "GameLogicProcessor: GameLogicComponent " << hdl << " logic " << i << " failed to Init! " << endlog;
				}
			}
		}
		m_notInitialized.clear();

		for (GameLogic* l : m_onFrameStart){
			l->FrameStart();
		}
	}

	void GameLogicProcessor::FrameMiddle(){
		// LOGTO(Verbose) << "GameLogicProcessor::FrameMiddle\n" << endlog;

		for (GameLogic* l : m_onFrameMiddle){
			l->FrameMiddle();
		}
	}

	void GameLogicProcessor::FrameEnd(){
		// LOGTO(Verbose) << "GameLogicProcessor::FrameEnd\n" << endlog;

		for (GameLogic* l : m_onFrameEnd){
			l->FrameEnd();
		}
	}

	ComponentHandle GameLogicProcessor::CreateComponent(EntityHandle entity)
	{
		ComponentHandle hdl = components.newComponent(entity, m_e_sys);
		m_notInitialized.emplace_back(hdl);

		return hdl;
	}

	void GameLogicProcessor::DestroyComponent(ComponentHandle component)
	{
		auto& pieces = components.getComponent(component)->m_gamelogics;
		for (GameLogic* logic : pieces)
		{
			GameLogic::RunEvents ev = logic->getRunEvents();
			if (ev & GameLogic::RunEvents::EFrameStart){
				m_onFrameStart.erase(std::find(m_onFrameStart.begin(), m_onFrameStart.end(), logic));
			}
			if (ev & GameLogic::RunEvents::EFrameMiddle){
				m_onFrameMiddle.erase(std::find(m_onFrameMiddle.begin(), m_onFrameMiddle.end(), logic));
			}
			if (ev & GameLogic::RunEvents::EFrameEnd){
				m_onFrameEnd.erase(std::find(m_onFrameEnd.begin(), m_onFrameEnd.end(), logic));
			}

			logic->End();
		}

		components.removeComponent(component);
	}

	Component* GameLogicProcessor::getComponent(ComponentHandle hdl)
	{
		return components.getComponent(hdl);
	}

	std::vector<ComponentHandle>& GameLogicProcessor::getComponents()
	{
		return components.getValidComponents();
	}


}