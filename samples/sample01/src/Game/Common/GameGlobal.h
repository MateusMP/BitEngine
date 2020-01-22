#pragma once

#include <BitEngine/Core/VideoSystem.h>
#include <BitEngine/Core/Logger.h>
#include <BitEngine/Core/Memory.h>
#include <BitEngine/Core/ECS/EntitySystem.h>

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

struct PlayerControlComponent : public BitEngine::Component<PlayerControlComponent>
{
    float movH, movV;

    BitEngine::ComponentRef<BitEngine::Transform2DComponent> transform2d;
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

    BitEngine::ComponentRef<PlayerControlComponent> playerControl;	
};

struct UserRequestQuitGame {
    GameQuitType quitType;
};