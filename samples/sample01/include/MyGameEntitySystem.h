#pragma once

#include <bitengine/Core/GameEngine.h>

#include <bitengine/Core/ResourceSystem.h>
#include <bitengine/Core/Graphics/Sprite2D.h>

#include "Common/GameGlobal.h"

#define ADD_COMPONENT_ERROR(x) \
		if (!(x).isValid()) {		\
			LOG(GameLog(), BE_LOG_ERROR) << "ADD COMPONENT FAILED FOR: " #x;	\
		abort();}


class MyGameEntitySystem : public BitEngine::EntitySystem
{
public:
	MyGameEntitySystem(BitEngine::GameEngine* ge)
		: EntitySystem(ge)
	{
		using namespace BitEngine;

		// Register components - will automatically register message ids
		RegisterComponent<GameLogicComponent>();
		RegisterComponent<Transform2DComponent>();
		RegisterComponent<Transform3DComponent>();
		RegisterComponent<Camera2DComponent>();
		RegisterComponent<Camera3DComponent>();
		RegisterComponent<RenderableMeshComponent>();
		//RegisterComponent<Sprite2DComponent>();
		RegisterComponent<Sprite2DComponent>(new ComponentHolder<Sprite2DComponent>(ge));
		RegisterComponent<SceneTransform2DComponent>();

		// Create entity system processors:
		t2p = new Transform2DProcessor(getEngine()->getMessenger());
		t3p = new Transform3DProcessor(getEngine()->getMessenger());
		cam2Dprocessor = new Camera2DProcessor(t2p);
		cam3Dprocessor = new Camera3DProcessor(t3p);
		glp = new GameLogicProcessor(getEngine()->getMessenger());
		rmp = new RenderableMeshProcessor(getEngine()->getMessenger());

		/// Pipeline 0
		RegisterComponentProcessor(0, glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameStart);

		RegisterComponentProcessor(0, glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameMiddle);

		RegisterComponentProcessor(0, t2p, (ComponentProcessor::processFunc)&Transform2DProcessor::Process);
		RegisterComponentProcessor(0, t3p, (ComponentProcessor::processFunc)&Transform3DProcessor::Process);
		RegisterComponentProcessor(0, cam2Dprocessor, (ComponentProcessor::processFunc)&Camera2DProcessor::Process);
		RegisterComponentProcessor(0, cam3Dprocessor, (ComponentProcessor::processFunc)&Camera3DProcessor::Process);
		RegisterComponentProcessor(0, glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameEnd);
	}

	///

	// Processors
	BitEngine::Transform2DProcessor *t2p;
	BitEngine::Transform3DProcessor *t3p;
	BitEngine::Camera2DProcessor *cam2Dprocessor;
	BitEngine::Camera3DProcessor *cam3Dprocessor;
	BitEngine::RenderableMeshProcessor *rmp;
	BitEngine::GameLogicProcessor *glp;
	BitEngine::Sprite2DRenderer *spr2D;
};
