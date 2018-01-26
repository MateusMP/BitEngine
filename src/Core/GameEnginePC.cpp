#include <stdio.h>
#include <stdlib.h>

#include "bitengine/Core/GameEnginePC.h"
#include "bitengine/Core/Logger.h"

namespace BitEngine {

class UpdateSystemTask : public Task
{
	public:
		UpdateSystemTask(System* s)
			: Task(Task::TaskMode::REPEAT_ONCE_PER_FRAME_REQUIRED, Task::Affinity::MAIN), sys(s) {}

		void run() 
		{
			sys->Update();
		}

		bool finished()
		{
			return false;
		}

	private:
		System* sys;
};

class MessengerDispatchTask : public Task
{
	public:
		MessengerDispatchTask(Messenger* m)
				: Task(Task::TaskMode::REPEAT_ONCE_PER_FRAME_REQUIRED, Task::Affinity::MAIN), msgr(m)
		{}

		void run()
		{
			msgr->dispatch();
		}

	private:
		Messenger* msgr;
};


GameEnginePC::GameEnginePC(ConfigurationLoader* _configLoader, ResourceLoader* _resourceLoader, Messenger* _messenger)
	: configurationLoader(_configLoader), resourceLoader(_resourceLoader), messenger(_messenger), taskManager(_messenger)
{
}

GameEnginePC::~GameEnginePC()
{
	delete configurationLoader;
	delete resourceLoader;
}

void GameEnginePC::stopRunning()
{
	taskManager.stop();
}

void GameEnginePC::addSystem(System *sys)
{
	const auto &it = systemsMap.find(sys->getName());
	if (it == systemsMap.end())
	{
		systems.emplace_back(sys);
		systemsMap.emplace(sys->getName(), sys);
	}
	else
	{
		LOG(EngineLog, BE_LOG_ERROR) << "System already added: " << sys->getName();
	}
}

System* GameEnginePC::getSystem(const std::string& name)
{
	const auto &it = systemsMap.find(name);
	if (it == systemsMap.end())
	{
		return nullptr;
	}
	else
	{
		return it->second;
	}
}

bool GameEnginePC::initSystems()
{	
	LOG(EngineLog, BE_LOG_INFO) << "Initializing " << systemsMap.size() << " systems ";
    for (System* s : systems)
    {
        if (!s->Init())
        {
            LOG(EngineLog, BE_LOG_INFO) << "System " << s->getName() << " failed to initialize!\n";
            return false;
        }
		else
		{
			taskManager.addTask(std::make_shared<UpdateSystemTask>(s));
		}

		systemsToShutdown.push_back(s);
    }
	
	LOG(EngineLog, BE_LOG_INFO) << "All systems set!";

    return true;
}

void GameEnginePC::shutdownSystems()
{
	LOG(EngineLog, BE_LOG_INFO) << "Shutitng down systems...";
	
	// Shutdown in reverse order
	for (auto it = systemsToShutdown.rbegin(); it != systemsToShutdown.rend(); ++it){
		LOG(EngineLog, BE_LOG_INFO) << "Shutting down " << (*it)->getName();
        (*it)->Shutdown();
		LOG(EngineLog, BE_LOG_INFO) << "done " << (*it)->getName();

		delete *it;
    }

	systems.clear();
	systemsMap.clear();
	systemsToShutdown.clear();
	
	LOG(EngineLog, BE_LOG_INFO) << "Systems released!";
}

bool GameEnginePC::run()
{
    LOG(EngineLog, BE_LOG_INFO) << "Run Started";
   
	LOG(EngineLog, BE_LOG_INFO) << "Loadiging system configurations...";
	configurationLoader->loadConfigurations(configuration);

	taskManager.init();
	
	taskManager.addTask(std::make_shared<MessengerDispatchTask>(messenger));

    if ( initSystems() )
    {
		Time::ResetTicks();
		
		taskManager.update();
    }

    shutdownSystems();

	taskManager.shutdown();

    return true;
}

}
