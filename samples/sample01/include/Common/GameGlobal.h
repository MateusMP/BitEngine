#pragma once

#include <bitengine/Core/Logger.h>
#include <bitengine/Core/Memory.h>

extern BitEngine::Logger* GameLog();

struct MainMemory;
struct GameState;

#define GAME_UPDATE(name) bool32 name(MainMemory* gameMemory)
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


enum GameQuitType {
    CLOSE_WINDOW
};

// The game expect that all of these are initialized before the gameUpdate is called
struct MainMemory {
	
	void* memory;
	ptrsize memorySize;

	GameUpdate* gameUpdate;

	BitEngine::ResourceManager* spriteManager;
	BitEngine::ResourceManager* shaderManager;
	BitEngine::ResourceManager* textureManager;
	
	BitEngine::Messenger* messenger;
	BitEngine::EngineConfiguration *engineConfig;
	BitEngine::TaskManager* taskManager;
};

struct GameState {

	bool32 initialized;
	bool32 running;

	BitEngine::ResourceLoader *resources;

	BitEngine::MemoryArena mainArena;
	BitEngine::MemoryArena resourceArena;
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

struct UserRequestQuitGame {
    GameQuitType quitType;
};