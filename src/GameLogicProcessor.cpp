#include "GameLogicProcessor.h"

namespace BitEngine{

	GameLogicProcessor::GameLogicProcessor()
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

	bool GameLogicProcessor::Init(BaseEntitySystem* es){
		e_sys = es;

		// GameLogicComponentType = es->getComponentType<GameLogicComponent>();
		// es->getHolder(GameLogicComponentType)->RegisterListener(this);
		RegisterListener(this); // Self register

		return true;
	}

	void GameLogicProcessor::Stop()
	{
		UnregisterListener(this);
	}

	void GameLogicProcessor::FrameStart()
	{
		// LOGTO(Verbose) << "GameLogicProcessor::FrameStart\n" << endlog;

		// Initialize all components not yet initialized
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

		// Run FrameStart on all components
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

	void GameLogicProcessor::OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		GameLogicComponent* glc = components.getComponent(component);
		glc->e_sys = e_sys;

		m_notInitialized.emplace_back(component);
	}

	void GameLogicProcessor::OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		GameLogicComponent* comp = components.getComponent(component);
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

	ComponentHandle GameLogicProcessor::AllocComponent()
	{
		return components.newComponent();
	}

	void GameLogicProcessor::DeallocComponent(ComponentHandle component)
	{
		components.removeComponent(component);
	}

	Component* GameLogicProcessor::getComponent(ComponentHandle hdl)
	{
		return components.getComponent(hdl);
	}

	const std::vector<ComponentHandle>& GameLogicProcessor::getComponents() const
	{
		return components.getValidComponents();
	}


}