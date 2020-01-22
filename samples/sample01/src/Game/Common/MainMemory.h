#include <BitEngine/Core/Memory.h>
#include <BitEngine/Core/Math.h>
#include <BitEngine/Core/Graphics/Sprite2DRenderer.h>

namespace BitEngine
{
class VideoSystem;
class Sprite2DRenderer;
class EngineConfiguration;
}

enum Command {
    SCENE_BEGIN,
    RENDER_SPRITE,
    SCENE_2D,
    SPRITE_BATCH_2D,
    SCENE_3D_BATCH,

};

struct RenderSpriteCommand {
    BitEngine::Material* material;
    BitEngine::Vec4 uvs;
    BitEngine::Mat3 transform;
};

struct SceneBeginCommand {
    u32 renderWidth;
    u32 renderHeight;
};

struct Render2DSceneCommand {
    BitEngine::Sprite2DRenderer* renderer;
};

struct RenderSpriteBatch2DCommand {
    const std::vector<BitEngine::Sprite2DBatch>& batch2d;
    BitEngine::Mat4 viewProj;
};

struct Model3D {
    BitEngine::Mesh *mesh;
    BitEngine::Material* material;
    BitEngine::Mat4 transform;
};
struct Render3DBatchCommand {
    BitEngine::Mat4 projection;
    BitEngine::Mat4 view;
    Model3D* data;
    u32 count;
};

union CommandData {
    CommandData(SceneBeginCommand&& s) : sceneBegin(s) {
    }
    CommandData(RenderSpriteCommand&& s) : renderSprite(s) {
    }
    CommandData(Render2DSceneCommand&& s) : render2DScene(s) {
    }
    CommandData(RenderSpriteBatch2DCommand&& s) : renderSpriteBatch2D(s) {
    }
    CommandData(Render3DBatchCommand&& s) : batch3d(s) {
    }
    RenderSpriteBatch2DCommand renderSpriteBatch2D;
    Render2DSceneCommand render2DScene;
    SceneBeginCommand sceneBegin;
    RenderSpriteCommand renderSprite;
    Render3DBatchCommand batch3d;
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

    Model3D* pushModel3D(Render3DBatchCommand* command) {
        command->count += 1;
        return dataArena.push<Model3D>() - 1; // We are always one ahead.
    }

    void pushCommand(BitEngine::Sprite2DRenderer* renderer) {
        commandArena.push<RenderCommand>(RenderCommand{ Command::SCENE_2D, Render2DSceneCommand{ renderer } });
    }

    void pushCommand(const std::vector<BitEngine::Sprite2DBatch>& batch2d, const BitEngine::Mat4& viewProj) {
        commandArena.push<RenderCommand>(RenderCommand{ Command::SPRITE_BATCH_2D, RenderSpriteBatch2DCommand{ batch2d, viewProj } });
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

    RenderQueue* renderQueue;
};