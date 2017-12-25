#pragma once

#include <bitengine/core/GameEnginePC.h>
#include <bitengine/Core/Resources/SpriteManager.h>

class MyResourceSystem : public BitEngine::ResourceSystem
{
	public:
	MyResourceSystem(BitEngine::GameEngine* ge)
		: ResourceSystem(ge)
	{
		getEngine()->getMessenger()->registerListener<BitEngine::Input::MsgKeyboardInput>(this);
	}

	void onMessage(const BitEngine::Input::MsgKeyboardInput& msg)
	{
		if (msg.key == GLFW_KEY_R && msg.keyAction == BitEngine::Input::KeyAction::PRESS && msg.keyMod == BitEngine::Input::KeyMod::CTRL_SHIFT)
		{
			LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Reloading index";
			getResourceLoader()->loadIndex("data/main.idx");
		}
	}

	bool Init() override {
		getResourceLoader()->registerResourceManager("SPRITE", &spriteManager);

		ResourceSystem::Init();

		return getResourceLoader()->loadIndex("data/main.idx");
	}

	private:
		BitEngine::SpriteManager spriteManager;
};