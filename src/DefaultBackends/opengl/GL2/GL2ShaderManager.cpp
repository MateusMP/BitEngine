#include "bitengine/Common/MathUtils.h"
#include "bitengine/Core/Logger.h"

#include "bitengine/Core/TaskManager.h"
#include "bitengine/DefaultBackends/opengl/GL2/GL2Shader.h"
#include "bitengine/DefaultBackends/opengl/GL2/GL2ShaderManager.h"

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
				std::string sourceType = dr.meta->properties["source_type"].getValueString();

				if (sourceType.compare("VERTEX") == 0)
				{
					LOG(EngineLog, BE_LOG_VERBOSE) << "Loading vertex piece " << dr.meta->resourceName;
					shader->includeSource(GL_VERTEX_SHADER, dr.data);
				}
				else if (sourceType.compare("FRAGMENT") == 0)
				{
					LOG(EngineLog, BE_LOG_VERBOSE) << "Loading fragment piece " << dr.meta->resourceName;
					shader->includeSource(GL_FRAGMENT_SHADER, dr.data);
				}
				else if (sourceType.compare("GEOMETRY") == 0)
				{
					LOG(EngineLog, BE_LOG_VERBOSE) << "Loading geometry piece " << dr.meta->resourceName;
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
			if (shader->gotAllShaderPiecesLoaded())
			{
				shader->init();
			}
		}
	}

	void initShadeDefinition(ShaderDataDefinition& def, ResourceMeta* meta)
	{
		ResourcePropertyContainer definition = meta->properties["gl2"]["definition"];

		int nDefs = definition.getNumberOfProperties();
		for (int i = 0; i < nDefs; ++i)
		{
			ResourcePropertyContainer container = definition[i];
			BitEngine::DataUseMode useMode = useModeFromString(container["mode"].getValueString());
			int instanced = container["instanced"].getValueInt();
			ShaderDataDefinition::DefinitionContainer& dataDefContainer = def.addContainer(useMode, instanced);
			
			ResourcePropertyContainer dataDefResource = container["defs"];
			int nDataDef = dataDefResource.getNumberOfProperties();

			for (int j = 0; j < nDataDef; ++j)
			{
				ResourcePropertyContainer dataDef = dataDefResource[j];
				dataDefContainer.addDataDef(
					dataDef["name"].getValueString(),
					dataTypeFromString(dataDef["data"].getValueString()),
					dataDef["size"].getValueInt());
			}
		}

	}

	ResourceLoader::RawResourceTask GL2ShaderManager::loadShaderSource(ResourcePropertyContainer& rpc, GL2Shader* shader)
	{
		const std::string file = rpc.getValueString();
		ResourceMeta* shaderSourceMeta = loader->findMeta(file);
		if (shaderSourceMeta != nullptr)
		{
			sourceShaderRelation.emplace(shaderSourceMeta, shader);
			return loadRawData(loader, shaderSourceMeta);
		}
		else
		{
			LOG(EngineLog, BE_LOG_ERROR) << "Couldn't load shader source: " << file;
		}
		return nullptr;
	}

	void GL2ShaderManager::makeFullLoad(ResourceMeta* meta, GL2Shader* shader)
	{
		initShadeDefinition(shader->getDefinition(), meta);
		// Load source files
		int expectedTypes = 0;
		auto gl2Props = meta->properties["gl2"];
		auto vertexProp = gl2Props["vertex"];
		auto fragmentProp = gl2Props["fragment"];
		auto geometryProp = gl2Props["geometry"];
		if (vertexProp.isValid() && !vertexProp.getValueString().empty()) {
			expectedTypes++;
		}
		if (fragmentProp.isValid()) {
			expectedTypes++;
		}
		if (geometryProp.isValid()) {
			expectedTypes++;
		}
		shader->setExpectedShaderSources(expectedTypes);
		std::shared_ptr<ShaderSourceLoader> shaderSourceLoader = std::make_shared<ShaderSourceLoader>(this, shader);
		if (vertexProp.isValid()) {
			shaderSourceLoader->addDependency(loadShaderSource(vertexProp, shader));
		}
		if (fragmentProp.isValid()) {
			shaderSourceLoader->addDependency(
					loadShaderSource(fragmentProp, shader));
		}
		if (geometryProp.isValid()) {
			shaderSourceLoader->addDependency(
					loadShaderSource(geometryProp, shader));
		}
		taskManager->addTask(shaderSourceLoader);
	}

	BaseResource* GL2ShaderManager::loadResource(ResourceMeta* meta)
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
