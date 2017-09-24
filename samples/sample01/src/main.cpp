
#include <string>

#include <bitengine/bitengine.h>
#include <bitengine/core/GameEnginePC.h>
#include <bitengine/core/Resources/DevResourceLoader.h>
#include <bitengine/DefaultBackends/opengl/GL2/GL2Driver.h>
#include <bitengine/DefaultBackends/glfw/GLFW_Backend.h>

#include "BasicTypes.h"
#include "Player.h"
#include "GameWorld.h"
#include "Overworld.h"

#include "MyGameSystem.h"
#include "MyResourceSystem.h"

class MyGame : public BitEngine::GameEnginePC
{
public:
    MyGame()
        : GameEnginePC(new BitEngine::EngineConfigurationFileLoader("config.ini"),  new BitEngine::DevResourceLoader(this), new BitEngine::GL2Driver(this))
    {
		getMessenger()->registerListener<BitEngine::MsgWindowClosed>(this);
		getMessenger()->registerListener<BitEngine::MsgFrameStart>(this);
		
		createSystems();
    }

	~MyGame()
	{
		getMessenger()->unregisterListener<BitEngine::MsgWindowClosed>(this);
	}

	void onMessage(const BitEngine::MsgWindowClosed& msg)
	{
		stopRunning();
	}

	void onMessage(const BitEngine::MsgFrameStart& msg)
	{
		//LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Frame Start";
	}

protected:
	void createSystems() 
	{
		using namespace BitEngine;
		
		VideoSystem* videoSys = new GLFW_VideoSystem(this);
		InputSystem* inputSys = new GLFW_InputSystem(this);
		ResourceSystem* resourceSys = new MyResourceSystem(this);
		CommandSystem* cmdSys = new CommandSystem(this);
        
        // TODO: Join these in a single game system
		MyGameSystem* mySys = new MyGameSystem(this, cmdSys, videoSys);
		
		///////
		addSystem(videoSys);
		addSystem(inputSys);
		addSystem(resourceSys);
		addSystem(cmdSys);
        
		addSystem(mySys);
	}

	private:
		BitEngine::DevResourceLoader *fileLoader;

};

BitEngine::Logger* GameLog()
{
	static BitEngine::Logger log("GameLog", BitEngine::EngineLog);
	return &log;
}

//

int main()
{
	LOG_FUNCTION_TIME(GameLog());
    MyGame game;
    game.run();
    return 0;
}
