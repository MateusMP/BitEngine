#include "BitEngine/Common/MathUtils.h"
#include "BitEngine/Core/Logger.h"

#include "BitEngine/Core/TaskManager.h"
#include "Platform/opengl/GL2/GL2Shader.h"
#include "Platform/opengl/GL2/GL2ShaderManager.h"

#include "BitEngine/Core/Resources/DevResourceLoader.h"
#include "BitEngine/Core/IO/File.h"

namespace BitEngine {


class ShaderSourceLoader : public Task {
public:
    ShaderSourceLoader(GL2ShaderManager* _manager, GL2Shader* _shader, const GL2ShaderInfo& def)
        : Task(Task::TaskMode::REPEAT_ONCE_PER_FRAME, Task::Affinity::BACKGROUND), manager(_manager), shader(_shader), shaderInfo(def)
    {}

    bool sourceReady(const RR<File>& f) {
        return (f.isValid() && f->ready) || !f.isValid();
    }

    // Inherited via Task
    virtual void run() override
    {
        BE_PROFILE_FUNCTION();
        bool readyForGPU = sourceReady(shaderInfo.vertex);
        readyForGPU &= sourceReady(shaderInfo.fragment);
        readyForGPU &= sourceReady(shaderInfo.geometry);

        if (readyForGPU)
        {
            manager->sendToGPU(shader, shaderInfo);
            stopRepeating();
        }
    }

private:
    GL2ShaderManager* manager;
    GL2Shader* shader;
    const GL2ShaderInfo shaderInfo;
};

//

GL2ShaderManager::GL2ShaderManager(TaskManager* tm)
    : taskManager(tm), loader(nullptr)
{
    ramInUse = 0;
    gpuMemInUse = 0;
}

GL2ShaderManager::~GL2ShaderManager()
{
    for (GL2Shader& shader : shaders.getResources())
    {
        shader.releaseShader();
    }
};

bool GL2ShaderManager::init()
{
    // Init error texture
    //GL2Shader& basicShader = shaders.getResourceAt(0);

    return true;
}

void GL2ShaderManager::shutdown() {
    for (GL2Shader& s : shaders.getResources()) {
        s.releaseShader();
    }
}

void GL2ShaderManager::update()
{
    BE_PROFILE_FUNCTION();
    ToLoad toload;
    while (resourceLoaded.tryPop(toload))
    {
        GLuint pieces[3];
        u32 npieces = 0;
        if (toload.info.vertex)
        {
            LOG(EngineLog, BE_LOG_VERBOSE) << "Loading vertex piece for for shader " << toload.shader->getMeta()->getNameId();
            pieces[npieces] = toload.shader->attachSource(GL_VERTEX_SHADER, toload.info.vertex->data, toload.info.vertex->size);
            ++npieces;
        }
        
        if (toload.info.fragment)
        {
            LOG(EngineLog, BE_LOG_VERBOSE) << "Loading fragment piece for shader " << toload.shader->getMeta()->getNameId();
            pieces[npieces] = toload.shader->attachSource(GL_FRAGMENT_SHADER, toload.info.fragment->data, toload.info.fragment->size);
            ++npieces;
        }
        
        if (toload.info.geometry)
        {
            LOG(EngineLog, BE_LOG_VERBOSE) << "Loading geometry piece for shader " << toload.shader->getMeta()->getNameId();
            pieces[npieces] = toload.shader->attachSource(GL_GEOMETRY_SHADER, toload.info.geometry->data, toload.info.geometry->size);
            ++npieces;
        }

        toload.shader->init();
    }
}

void GL2ShaderManager::makeFullLoad(ResourceMeta* meta, GL2Shader* shader)
{
    if (shader->reload) {
        shader->reload();
    }
}

Shader* GL2ShaderManager::loadResource(ResourceMeta* meta, PropertyHolder* props)
{
    GL2Shader* shader = shaders.findResource(meta);

    if (shader == nullptr)
    {
        u16 id = shaders.addResource(meta);
        shader = shaders.getResourceAddress(id);

        new (shader) GL2Shader(meta);
        {
            // Init definition
            GL2ShaderInfo info = { &shader->getDefinition() };
            props->readObject("gl2", &info);

            std::shared_ptr<ShaderSourceLoader> loadTask = std::make_shared<ShaderSourceLoader>(this, shader, info);
            taskManager->addTask(loadTask);
        }

    }

    return shader;
}

void GL2ShaderManager::reloadResource(BaseResource* resource)
{
    GL2Shader* shader = static_cast<GL2Shader*>(resource);
    shader->releaseShader();
    makeFullLoad(resource->getMeta(), shader);
}

void GL2ShaderManager::sendToGPU(GL2Shader* shader, const GL2ShaderInfo& info) {
    resourceLoaded.push(ToLoad{ shader, info });
}
}
