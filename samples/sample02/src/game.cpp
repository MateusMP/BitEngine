
#include <BitEngine/Core/Logger.h>

#include <bitengine/bitengine.h>
#include <bitengine/Core/Messenger.h>
#include <bitengine/Core/GeneralTaskManager.h>
#include <BitEngine/Core/Memory.h>

#include "Common/GameGlobal.h"
#include "Common/MainMemory.h"
#include "MyGame.h"

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