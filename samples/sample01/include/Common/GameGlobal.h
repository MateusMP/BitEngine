#pragma once

#include <bitengine/Core/Logger.h>
#include <bitengine/Core/Memory.h>

extern BitEngine::Logger* GameLog();

struct GameMemory;
struct GameState;

#define GAME_UPDATE(name) bool32 name(GameMemory* gameMemory)
typedef GAME_UPDATE(GameUpdate);

class MyGameEntitySystem;
class Player;
class PlayerController;

class UserGUI;
class GameWorld;


enum GAME_STATES {
	MENU,
	GAMEPLAY,
};

enum TEST_COMMANDS {
	RIGHT,
	LEFT,
	UP,
	DOWN,

	CLICK,

	RELOAD_SHADERS,

	end
};

struct GameMemory {
	
	void* memory;
	ptrsize memorySize;

	GameUpdate* gameUpdate;
	
	BitEngine::Messenger* messenger;
	BitEngine::ResourceLoader *resources;
	BitEngine::EngineConfiguration *engineConfig;
	BitEngine::TaskManager* taskManager;


};

struct GameState {

	bool32 initialized;
	bool32 running;

	BitEngine::MemoryArena permanentArena;
	BitEngine::MemoryArena entityArena;

	MyGameEntitySystem* entitySystem;

	Player *selfPlayer;
	PlayerController* playerController;

	UserGUI* m_userGUI;
	GameWorld* m_world; //!< Current active world
	
};

struct RenderEvent {
	GameState* state;
};