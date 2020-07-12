#pragma once

#include <BitEngine/Core/VideoSystem.h>
#include <BitEngine/Core/Logger.h>
#include <BitEngine/Core/Memory.h>

extern BitEngine::Logger* GameLog();

struct MainMemory;
struct GameState;
class MyGame;

class MyGameEntitySystem;
class PlayerCamera;

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
