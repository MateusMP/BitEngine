#include <cr/cr.h>
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

bool GAME_SETUP(MainMemory* mainMemory) {
    BitEngine::EngineLog = mainMemory->logger;
    BitEngine::Profiling::SetInstance(mainMemory->profiler);
    ImGui::SetCurrentContext((ImGuiContext*)mainMemory->imGuiContext);
    LOG(GameLog(), BE_LOG_INFO) << "Game Setup!";
    game = new MyGame(mainMemory);
    mainMemory->userdata = game;
    return game != nullptr;
}

bool GAME_UPDATE(MainMemory* mainMemory) {
    return game->update();
}

bool GAME_SHUTDOWN(MainMemory* mainMemory) {
    LOG(GameLog(), BE_LOG_INFO) << "Game Shutdown!";
    delete game;
    return true;
}


CR_EXPORT int cr_main(cr_plugin *ctx, cr_op operation) {
    assert(ctx);
    MainMemory* g_data = (MainMemory *)ctx->userdata;

    switch (operation) {
        case CR_LOAD:
            GAME_SETUP(g_data);
            LOG(GameLog(), BE_LOG_INFO) << "Game loading, version " << ctx->version << " errors: " << ctx->failure;
            return 0;
        case CR_UNLOAD:
            LOG(GameLog(), BE_LOG_INFO) << "Game Unload!";
            GAME_SHUTDOWN(g_data);
            // if needed, save stuff to pass over to next instance
            return 0;
        case CR_CLOSE:
            LOG(GameLog(), BE_LOG_INFO) << "Game Close!";
            GAME_SHUTDOWN(g_data);
            return 0;
        case CR_STEP:
            return GAME_UPDATE(g_data);
    }

    return 0;
}