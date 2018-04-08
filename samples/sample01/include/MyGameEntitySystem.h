#pragma once

#include <bitengine/Core/GameEngine.h>

#include <bitengine/Core/ResourceSystem.h>
#include <bitengine/Core/Graphics/Sprite2D.h>
#include <bitengine/Core/ECS/Camera2DProcessor.h>
#include <bitengine/Core/ECS/Camera3DProcessor.h>
#include <bitengine/Core/ECS/RenderableMeshProcessor.h>
#include <bitengine/Core/ECS/GameLogicProcessor.h>

#include "Graphics/Shader3DProcessor.h"

#include "Common/GameGlobal.h"

#define ADD_COMPONENT_ERROR(x) \
		if (!(x).isValid()) {		\
			LOG(GameLog(), BE_LOG_ERROR) << "ADD COMPONENT FAILED FOR: " #x;	\
		abort();}

class MyGameEntitySystem : public BitEngine::EntitySystem
{
public:
	MyGameEntitySystem(BitEngine::ResourceLoader* loader, BitEngine::Messenger* messenger, BitEngine::MemoryArena* entityMemory)
		: EntitySystem(messenger), 
		t2p(messenger), t3p(messenger), 
		cam2Dprocessor(&t2p), cam3Dprocessor(&t3p),
		rmp(messenger),
		glp(messenger), spr2D(messenger, loader)
		//sh3D(&t3p)
	{
		using namespace BitEngine;

		// Register components - will automatically register message ids
		RegisterComponent<GameLogicComponent>();
		RegisterComponent<Transform2DComponent>();
		RegisterComponent<Transform3DComponent>();
		RegisterComponent<Camera2DComponent>();
		RegisterComponent<Camera3DComponent>();
		RegisterComponent<RenderableMeshComponent>();
		RegisterComponent<Sprite2DComponent>();
		RegisterComponent<SceneTransform2DComponent>();

		/// Pipeline 0
		RegisterComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameStart);

		RegisterComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameMiddle);

		RegisterComponentProcessor(0, &t2p, (ComponentProcessor::processFunc)&Transform2DProcessor::Process);
		RegisterComponentProcessor(0, &t3p, (ComponentProcessor::processFunc)&Transform3DProcessor::Process);
		RegisterComponentProcessor(0, &cam2Dprocessor, (ComponentProcessor::processFunc)&Camera2DProcessor::Process);
		RegisterComponentProcessor(0, &cam3Dprocessor, (ComponentProcessor::processFunc)&Camera3DProcessor::Process);
		RegisterComponentProcessor(0, &glp, (ComponentProcessor::processFunc)&GameLogicProcessor::FrameEnd);

		InitComponentProcessor(&spr2D);
	}

	// Processors
	BitEngine::Transform2DProcessor t2p;
	BitEngine::Transform3DProcessor t3p;
	BitEngine::Camera2DProcessor cam2Dprocessor;
	BitEngine::Camera3DProcessor cam3Dprocessor;
	BitEngine::RenderableMeshProcessor rmp;
	BitEngine::GameLogicProcessor glp;
	BitEngine::Sprite2DRenderer spr2D;

	//Shader3DProcessor sh3D;
};
