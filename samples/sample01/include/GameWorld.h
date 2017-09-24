#pragma once

#include "MyGameEntitySystem.h"

class GameWorld
{
public:
	GameWorld(MyGameEntitySystem* es)
		: m_ES(es)
	{
	}
	virtual ~GameWorld(){};

	// Prepare all resources
	virtual bool Init() = 0;
	// Releases all resources
	virtual void Shutdown() = 0;

	void setActiveCamera(const BitEngine::ComponentRef<BitEngine::Camera3DComponent>& camera)
	{
		activeCamera = camera;
	}

	const BitEngine::ComponentRef<BitEngine::Camera3DComponent> getActiveCamera() const
	{
		return activeCamera;
	}

	virtual int AddPlayer(Player* player) = 0;

	// Begin world game
	virtual void Start() = 0;

protected:
	MyGameEntitySystem* m_ES;
	BitEngine::ComponentRef<BitEngine::Camera3DComponent> activeCamera;

};
