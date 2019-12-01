#include "BitEngine/Common/MathUtils.h"
#include "BitEngine/Core/Logger.h"

#include "BitEngine/Core/TaskManager.h"
#include "Platform/opengl/GL2/GL2Shader.h"
#include "Platform/opengl/GL2/GL2ShaderManager.h"

#include "BitEngine/Core/Resources/DevResourceLoader.h"

namespace BitEngine {

    class ShaderSourceLoader : public Task {
    public:
        ShaderSourceLoader(GL2ShaderManager* _manager, GL2Shader* _shader)
            : Task(Task::TaskMode::NONE, Task::Affinity::BACKGROUND), manager(_manager), shader(_shader)
        {}

        // Inherited via Task
        virtual void run() override
        {
            for (const TaskPtr& task : getDependencies())
            {
                ResourceLoader::RawResourceLoaderTask* rawTask = static_cast<ResourceLoader::RawResourceLoaderTask*>(task.get());
                auto& dr = rawTask->getData();
                const std::string& sourceType = dr.meta->properties["source_type"].get_ref<std::string&>();

                if (sourceType.compare("VERTEX") == 0)
                {
                    LOG(EngineLog, BE_LOG_VERBOSE) << "Loading vertex piece for " << dr.meta->getNameId();
                    shader->includeSource(GL_VERTEX_SHADER, dr.data);
                }
                else if (sourceType.compare("FRAGMENT") == 0)
                {
                    LOG(EngineLog, BE_LOG_VERBOSE) << "Loading fragment piece " << dr.meta->getNameId();
                    shader->includeSource(GL_FRAGMENT_SHADER, dr.data);
                }
                else if (sourceType.compare("GEOMETRY") == 0)
                {
                    LOG(EngineLog, BE_LOG_VERBOSE) << "Loading geometry piece " << dr.meta->getNameId();
                    shader->includeSource(GL_GEOMETRY_SHADER, dr.data);
                }
            }
            manager->sendToGPU(shader);
        }

    private:
        GL2ShaderManager* manager;
        GL2Shader* shader;
    };

    //

    ResourceLoader::RawResourceTask GL2ShaderManager::loadShaderSource(DevResourceLoader* loader, const std::string& file, GL2Shader* shader)
    {
        ResourceMeta* shaderSourceMeta = loader->findMeta(file);
        if (shaderSourceMeta != nullptr)
        {
            sourceShaderRelation.emplace(shaderSourceMeta, shader);
            return loader->loadRawData(shaderSourceMeta);
        }
        else
        {
            LOG(EngineLog, BE_LOG_ERROR) << "Couldn't load shader source: " << file;
        }
        return nullptr;
    }

    void GL2ShaderManager::readJsonProperties(DevResourceLoader* devloader, nlohmann::json& props, ResourceManager* manager, BaseResource* resource) {
        GL2ShaderManager* mng = (GL2ShaderManager*)manager;
        GL2Shader* shader = (GL2Shader*)resource;
        // Init definition
        const nlohmann::json& gl2props = props["gl2"];
        const nlohmann::json& definition = gl2props["definition"];

        size_t nDefs = definition.size();
        for (int i = 0; i < nDefs; ++i)
        {
            const nlohmann::json& container = definition[i];
            BitEngine::DataUseMode useMode = useModeFromString(container["mode"].get<std::string>());
            int instanced = container["instanced"].get<int>();
            ShaderDataDefinition::DefinitionContainer& dataDefContainer = shader->getDefinition().addContainer(useMode, instanced);

            const nlohmann::json& dataDefResource = container["defs"];
            size_t nDataDef = dataDefResource.size();

            for (int j = 0; j < nDataDef; ++j)
            {
                const nlohmann::json& dataDef = dataDefResource[j];
                dataDefContainer.addDataDef(
                    dataDef["name"].get<std::string>(),
                    dataTypeFromString(dataDef["data"].get<std::string>()),
                    dataDef["size"].get<int>());
            }
        }


        // Load source files
        int expectedTypes = 0;
        auto vertexProp = gl2props.find("vertex");
        auto fragmentProp = gl2props.find("fragment");
        auto geometryProp = gl2props.find("geometry");

        shader->reload = std::function([=]() {
            std::shared_ptr<ShaderSourceLoader> loadTask = std::make_shared<ShaderSourceLoader>(mng, shader);

            if (vertexProp != gl2props.end() && (*vertexProp).size() > 0) {
                loadTask->addDependency(mng->loadShaderSource(devloader, (*vertexProp).get_ref<const std::string&>(), shader));
            }
            if (fragmentProp != gl2props.end() && (*fragmentProp).size() > 0) {
                loadTask->addDependency(mng->loadShaderSource(devloader, (*fragmentProp).get_ref<const std::string&>(), shader));
            }
            if (geometryProp != gl2props.end() && (*geometryProp).size() > 0) {
                loadTask->addDependency(mng->loadShaderSource(devloader, (*geometryProp).get_ref<const std::string&>(), shader));
            }
            mng->taskManager->addTask(loadTask);
        });
    }

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
        GL2Shader* shader;
        while (resourceLoaded.tryPop(shader))
        {
            shader->init();
        }
    }

    void GL2ShaderManager::makeFullLoad(ResourceMeta* meta, GL2Shader* shader)
    {
        if (shader->reload)
            shader->reload();
    }

    Shader* GL2ShaderManager::loadResource(ResourceMeta* meta)
    {
        GL2Shader* shader = shaders.findResource(meta);

        if (shader == nullptr)
        {
            u16 id = shaders.addResource(meta);
            shader = shaders.getResourceAddress(id);
            new (shader) GL2Shader(meta);

            makeFullLoad(meta, shader);
        }

        return shader;
    }

    void GL2ShaderManager::reloadResource(BaseResource* resource)
    {
        GL2Shader* shader = static_cast<GL2Shader*>(resource);
        shader->releaseShader();
        makeFullLoad(resource->getMeta(), shader);
    }

    void GL2ShaderManager::sendToGPU(GL2Shader* shader)
    {
        resourceLoaded.push(shader);
    }
}
