#pragma once

#include <bitengine/Core/VideoSystem.h>
#include <bitengine/Core/Logger.h>
#include <bitengine/Core/Memory.h>

extern BitEngine::Logger* GameLog();

struct MainMemory;
struct GameState;
class MyGame;

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

struct GameState {

	bool32 initialized;
	bool32 running;

	BitEngine::ResourceLoader *resources;

	BitEngine::MemoryArena mainArena;
	BitEngine::MemoryArena resourceArena;
	BitEngine::MemoryArena permanentArena;
	BitEngine::MemoryArena entityArena;

	MyGameEntitySystem* entitySystem;

	UserGUI* m_userGUI;
	GameWorld* m_world; //!< Current active world
	
};

struct UserRequestQuitGame {
    GameQuitType quitType;
};