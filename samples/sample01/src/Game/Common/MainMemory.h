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

enum Command {
    SCENE_BEGIN,
    RENDER_SPRITE,
    SCENE_2D,
    SPRITE_BATCH_2D,
    SCENE_3D_BATCH,

};

enum SpriteModes {
    DEFAULT_SPRITE = 0,
    TRANSPARENT_SPRITE,
    EFFECT_SPRITE
};

struct RenderSpriteCommand {
    BitEngine::Material* material;
    BitEngine::Vec4 uvs;
    BitEngine::Mat3 transform;
};

struct SceneBeginCommand {
    u32 renderWidth;
    u32 renderHeight;
    BitEngine::ColorRGBA color;
};

struct Model3D {
    BitEngine::Mesh *mesh;
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

struct Render3DBatchCommand {
    BitEngine::Mat4 projection;
    BitEngine::Mat4 view;
    Model3D* data;
    u32 count;

    Light light;
};
struct Render2DBatchCommand {
    BitEngine::Mat4 view;
    Sprite2D* data;
    u32 count;
};

union CommandData {
    CommandData(SceneBeginCommand&& s) : sceneBegin(s) {
    }
    CommandData(RenderSpriteCommand&& s) : renderSprite(s) {
    }
    CommandData(Render2DBatchCommand&& s) : batch2d(s) {
    }
    CommandData(Render3DBatchCommand&& s) : batch3d(s) {
    }
    SceneBeginCommand sceneBegin;
    RenderSpriteCommand renderSprite;
    Render3DBatchCommand batch3d;
    Render2DBatchCommand batch2d;
};

struct RenderCommand {
    Command command;
    CommandData data;
};

class RenderQueue {
public:
    RenderQueue(BitEngine::MemoryArena& memArena)
        : _fullarena(memArena) {
        commandArena.init((u8*)_fullarena.allocArrayDynamic<RenderCommand>(32), sizeof(RenderCommand)*32);
        ptrsize dataSize = _fullarena.remainingSize();
        dataArena.init((u8*)_fullarena.alloc(dataSize),dataSize);
    }
    void pushCommand(SceneBeginCommand command) {
        commandArena.push<RenderCommand>(RenderCommand{ Command::SCENE_BEGIN, SceneBeginCommand{command} });
    }

    Render3DBatchCommand* initRenderBatch3D() {
        RenderCommand* cmd = commandArena.push<RenderCommand>(RenderCommand{ Command::SCENE_3D_BATCH, Render3DBatchCommand{ } });
        Render3DBatchCommand *batch = &cmd->data.batch3d;
        batch->data = dataArena.push<Model3D>(); // Can't have an address without allocation a position
        batch->count = 0;
        return batch;
    }

    Render2DBatchCommand* initRenderBatch2D() {
        RenderCommand* cmd = commandArena.push<RenderCommand>(RenderCommand{ Command::SPRITE_BATCH_2D, Render2DBatchCommand{ } });
        Render2DBatchCommand *batch = &cmd->data.batch2d;
        batch->data = dataArena.push<Sprite2D>(); // Can't have an address without allocation a position
        batch->count = 0;
        return batch;
    }

    Sprite2D* pushSprite2D(Render2DBatchCommand* command, 
                            const BitEngine::Sprite* sprite, float alpha, int layer, const BitEngine::Material* material,
                            const BitEngine::Mat4& transform) {
        command->count += 1;
        Sprite2D* spr2d = dataArena.push<Sprite2D>() - 1;
        spr2d->sprite = sprite;
        spr2d->alpha = alpha;
        spr2d->layer = layer;
        spr2d->material = material;
        spr2d->transform = transform;
        return spr2d;
    }

    Model3D* pushModel3D(Render3DBatchCommand* command) {
        command->count += 1;
        return dataArena.push<Model3D>() - 1; // We are always one ahead.
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