
#include <BitEngine/Core/Logger.h>

#include <BitEngine/bitengine.h>
#include <BitEngine/Core/Messenger.h>
#include <BitEngine/Core/GeneralTaskManager.h>
#include <BitEngine/Core/Memory.h>

#include "Game/Common/GameGlobal.h"
#include "Game/Common/MainMemory.h"
#include "Game/MyGame.h"

static MyGame *game;
static BitEngine::Logger* gameLog;

static BitEngine::Logger* GameLog() {
    return gameLog;
}

extern "C" {
    __declspec(dllexport) bool GAME_SETUP(MainMemory* mainMemory) {
        gameLog = mainMemory->logger;
        BitEngine::EngineLog = gameLog;
        ImGui::SetCurrentContext((ImGuiContext*)mainMemory->imGuiContext);
        game = new MyGame(mainMemory);
        return game != nullptr;
    }
    __declspec(dllexport) bool GAME_UPDATE(MainMemory* mainMemory) {
        return game->update();
    }
    __declspec(dllexport) bool GAME_SHUTDOWN(MainMemory* mainMemory) {
        delete game;
        return true;
    }
}