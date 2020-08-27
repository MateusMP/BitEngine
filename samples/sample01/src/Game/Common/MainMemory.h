#pragma once

#include <BitEngine/Core/Memory.h>
#include <BitEngine/Core/TaskManager.h>
#include <BitEngine/Core/Math.h>
#include <BitEngine/Core/Graphics/Color.h>

namespace BitEngine
{
class VideoSystem;
class EngineConfiguration;
}

// ***** Generic Render Queue *****

struct RenderCommand {
    u32 command;
    void* data;

    template<typename T>
    T* dataAs() {
        return (T*)data;
    }
};

template<typename T>
struct BatchData {
    using Type = T;
    T* data;
    u32 count;
};


class RenderQueue {
public:
    RenderQueue(BitEngine::MemoryArena& memArena)
        : _fullarena(memArena) {
        commandArena.init((u8*)_fullarena.allocArrayDynamic<RenderCommand>(32), sizeof(RenderCommand) * 32);
        ptrsize dataSize = _fullarena.remainingSize();
        dataArena.init((u8*)_fullarena.alloc(dataSize), dataSize);
    }

    template<typename CmdType, typename ...Args>
    CmdType* pushCommand(Args&&... params) {
        CmdType* ptr = dataArena.push<CmdType>(std::forward<Args>(params)...);
        RenderCommand* command = commandArena.push<RenderCommand>(RenderCommand{ CmdType::CommandId, ptr });
        return ptr;
    }


    template<typename T, typename ...Args>
    T* initiateBatchCommand(Args&&... params) {
        T* cmd = pushCommand<T>(std::forward<Args>(params)...);
        cmd->batch.data = dataArena.push<typename decltype(T::batch)::Type>();
        cmd->batch.count = 0;
        return cmd;
    }

    template<typename T>
    auto pushBatchEntry(T* command) {
        using DType = typename decltype(T::batch)::Type;
        DType* entry = dataArena.push<DType>();
        command->batch.count += 1;
        return entry - 1;
    }

    RenderCommand* getCommands() {
        return (RenderCommand*)commandArena.base;
    }
    ptrsize getCommandsCount() {
        return commandArena.used / sizeof(RenderCommand);
    }

    void clear() {
        commandArena.clear();
        dataArena.clear();
    }

private:
    BitEngine::MemoryArena _fullarena;
    BitEngine::MemoryArena commandArena;
    BitEngine::MemoryArena dataArena;
};

// *****

enum Command {
    SCENE_BEGIN,
    SPRITE_BATCH_2D,
    SCENE_3D_BATCH,
};

constexpr const char* GetCommandName(const u32 cmd) {
    switch (cmd) {
    case SCENE_BEGIN:
        return "SCENE_BEGIN";
    case SPRITE_BATCH_2D:
        return "SPRITE_BATCH_2D";
    case SCENE_3D_BATCH:
        return "SCENE_3D_BATCH";
    }
    return "Unexpected command!";
}

enum SpriteModes {
    DEFAULT_SPRITE = 0,
    TRANSPARENT_SPRITE,
    EFFECT_SPRITE
};

enum class LightMode {
    SUNLIGHT,
    SPOT_LIGHT,
};

struct Light {
    LightMode mode;
    BitEngine::Vec3 position;
    BitEngine::ColorRGB color;
    BitEngine::Vec3 direction;
};

struct Model3D {
    BitEngine::Mesh* mesh;
    BitEngine::Material* material;
    BitEngine::Mat4 transform;
};

struct Sprite2D {
    const BitEngine::Sprite* sprite;
    int layer;
    float alpha;
    const BitEngine::Material* material;
    BitEngine::Mat3 transform;
};

// Commands

struct SceneBeginCommand {
    static constexpr Command CommandId = Command::SCENE_BEGIN;
    u32 renderWidth;
    u32 renderHeight;
    BitEngine::ColorRGBA color;
};

struct Render3DBatchCommand {
    static constexpr Command CommandId = Command::SCENE_3D_BATCH;
    BitEngine::Mat4 projection;
    BitEngine::Mat4 view;
    Light light;

    BatchData<Model3D> batch;
};
struct Render2DBatchCommand {
    static constexpr Command CommandId = Command::SPRITE_BATCH_2D;
    BitEngine::Mat4 view;

    BatchData<Sprite2D> batch;
};


// The game expect that all of these are initialized before the gameUpdate is called
struct MainMemory {

    void* memory;
    ptrsize memorySize;

    BitEngine::ResourceLoader* loader;
    BitEngine::ResourceManager* shaderManager;
    BitEngine::ResourceManager* textureManager;
    BitEngine::ResourceManager* spriteManager;

    BitEngine::VideoSystem* videoSystem;
    BitEngine::EngineConfiguration *engineConfig;
    BitEngine::TaskManager* taskManager;
    BitEngine::CommandSystem* commandSystem;

    BitEngine::Window* window;
    BitEngine::Messenger<BitEngine::ImGuiRenderEvent>* imGuiRender;
    void* imGuiContext;

    BitEngine::Logger* logger;
    BitEngine::Profiling::ChromeProfiler* profiler;

    RenderQueue* renderQueue;
};