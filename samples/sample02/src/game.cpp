
#include <BitEngine/Core/api.h>
#include <BitEngine/Core/Logger.h>

#include <BitEngine/bitengine.h>
#include <BitEngine/Core/Messenger.h>
#include <BitEngine/Core/GeneralTaskManager.h>
#include <BitEngine/Core/Memory.h>

#include "Game/Common/GameGlobal.h"
#include "Game/Common/MainMemory.h"
#include "Game/MyGame.h"

#include <BitEngine/Global/globals.cpp>

static MyGame *game;

BitEngine::Logger* GameLog() {
    return BitEngine::EngineLog;
}

extern "C" {
    BE_EXPORT bool GAME_SETUP(MainMemory* mainMemory) {
        BitEngine::EngineLog = mainMemory->logger;
        BitEngine::Profiling::SetInstance(mainMemory->profiler);
        ImGui::SetCurrentContext((ImGuiContext*)mainMemory->imGuiContext);
        game = new MyGame(mainMemory);
        return game != nullptr;
    }
    BE_EXPORT bool GAME_UPDATE(MainMemory* mainMemory) {
        return game->update();
    }
    BE_EXPORT bool GAME_SHUTDOWN(MainMemory* mainMemory) {
        delete game;
        return true;
    }
}