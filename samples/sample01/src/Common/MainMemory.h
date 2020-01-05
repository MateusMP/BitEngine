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

union CommandData {
    CommandData(SceneBeginCommand&& s) : sceneBegin(s) {
    }
    CommandData(RenderSpriteCommand&& s) : renderSprite(s) {
    }
    CommandData(Render2DSceneCommand&& s) : render2DScene(s) {
    }
    CommandData(RenderSpriteBatch2DCommand&& s) : renderSpriteBatch2D(s) {
    }
    RenderSpriteBatch2DCommand renderSpriteBatch2D;
    Render2DSceneCommand render2DScene;
    SceneBeginCommand sceneBegin;
    RenderSpriteCommand renderSprite;
};

struct RenderCommand {
    Command command;
    CommandData data;
};

class RenderQueue {
public:
    RenderQueue(BitEngine::MemoryArena& memArena)
        : arena(memArena) {

    }
    void pushCommand(SceneBeginCommand command) {
        arena.push<RenderCommand>(RenderCommand{ Command::SCENE_BEGIN, SceneBeginCommand{command} });
    }

    void pushCommand(BitEngine::Sprite2DRenderer* renderer) {
        arena.push<RenderCommand>(RenderCommand{ Command::SCENE_2D, Render2DSceneCommand{ renderer } });
    }

    void pushCommand(const std::vector<BitEngine::Sprite2DBatch>& batch2d, const BitEngine::Mat4& viewProj) {
        arena.push<RenderCommand>(RenderCommand{ Command::SPRITE_BATCH_2D, RenderSpriteBatch2DCommand{ batch2d, viewProj } });
    }

    RenderCommand* getCommands() {
        return (RenderCommand*)arena.base;
    }
    ptrsize getCommandsCount() {
        return arena.used / sizeof(RenderCommand);
    }

    void clear() {
        arena.clear();
    }
private:
    BitEngine::MemoryArena arena;
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