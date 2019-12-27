#include <BitEngine/Core/Memory.h>
#include <BitEngine/Core/Math.h>

namespace BitEngine
{
    class VideoSystem;
    class Sprite2DRenderer;
    class EngineConfiguration;
}

enum Command {
    SCENE_BEGIN,
    RENDER_SPRITE,
    SCENE_2D
};

struct RenderSpriteCommand {
    BitEngine::Material* material;
    BitEngine::Vec4 uvs;
    BitEngine::Mat3 transform;
};

struct SceneBeginCommand {
    u32 renderWidth;
    u32 renderHeight;
    BitEngine::Mat4 proj;
    BitEngine::Mat4 view;
};

struct Render2DSceneCommand {
    BitEngine::Sprite2DRenderer* renderer;
};

union CommandData {
    CommandData(SceneBeginCommand&& s) : sceneBegin(s) {
    }
    CommandData(RenderSpriteCommand&& s) : renderSprite(s) {
    }
    CommandData(Render2DSceneCommand&& s) : render2DScene(s) {
    }
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

    void pushCommand(BitEngine::Sprite2DRenderer* renderer) {
        arena.push<RenderCommand>(RenderCommand{Command::SCENE_2D, Render2DSceneCommand{ renderer } });
    }
    
    RenderCommand* getCommands() {
        return (RenderCommand*)arena.base;
    }
    u32 getCommandsCount() {
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