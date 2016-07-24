#include "Common/MathUtils.h"
#include "Core/Logger.h"

#include "DefaultBackends/opengl/GL2/GL2Shader.h"
#include "DefaultBackends/opengl/GL2/GL2ShaderManager.h"


namespace BitEngine {

	//

	GL2ShaderManager::GL2ShaderManager()
	{
		ramInUse = 0;
		gpuMemInUse = 0;
	}

	bool GL2ShaderManager::init()
	{
		// Init error texture
		GL2Shader& basicShader = shaders.getResourceAt(0);

		return true;
	}

	void GL2ShaderManager::update()
	{
		uint32 shaderIndex;
		while (resourceLoaded.tryPop(shaderIndex))
		{
			GL2Shader* shader = shaders.getResourceAddress(shaderIndex);
			if (shader->gotAllShaderPiecesLoaded())
			{
				shader->init();
			}
		}
	}

	BitEngine::DataUseMode useModeFromString(const std::string& str)
	{
		if (str.compare("VERTEX") == 0) {
			return DataUseMode::Vertex;
		}
		else if (str.compare("UNIFORM") == 0) {
			return DataUseMode::Uniform;
		}

		return DataUseMode::TotalModes;
	}

	BitEngine::DataType dataTypeFromString(const std::string& str)
	{
		if (str.compare("TEXTURE_1D") == 0) {
			return DataType::TEXTURE_1D;
		} else if (str.compare("TEXTURE_2D") == 0) {
			return DataType::TEXTURE_2D;
		} else if (str.compare("TEXTURE_3D") == 0) {
			return DataType::TEXTURE_3D;
		} else if (str.compare("LONG") == 0) {
			return DataType::LONG;
		} else if (str.compare("FLOAT") == 0) {
			return DataType::FLOAT;
		} else if (str.compare("VEC2") == 0) {
			return DataType::VEC2;
		} else if (str.compare("VEC3") == 0) {
			return DataType::VEC3;
		} else if (str.compare("VEC4") == 0) {
			return DataType::VEC4;
		} else if (str.compare("MAT3") == 0) {
			return DataType::MAT3;
		} else if (str.compare("MAT4") == 0) {
			return DataType::MAT4;
		}

		return DataType::INVALID_DATA_TYPE;
	}

	void initShadeDefinition(ShaderDataDefinition& def, ResourceMeta* meta)
	{
		ResourcePropertyContainer definition = meta->properties["definition"];

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

	void GL2ShaderManager::loadShaderSource(ResourcePropertyContainer& rpc, GL2Shader* shader)
	{
		const std::string file = rpc.getValueString();
		ResourceMeta* shaderSourceMeta = loader->findMeta(file);
		sourceShaderRelation.emplace(shaderSourceMeta, shader);
		loadRawData(loader, shaderSourceMeta, this);
	}

	BaseResource* GL2ShaderManager::loadResource(ResourceMeta* meta)
	{
		GL2Shader* shader = shaders.findResource(meta);

		if (shader == nullptr)
		{
			uint16 id = shaders.addResource(meta);
			shader = shaders.getResourceAddress(id);
			shader->setManagerResourceId(id);

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

			if (vertexProp.isValid()) {
				loadShaderSource(vertexProp, shader);
			}
			if (fragmentProp.isValid()) {
				loadShaderSource(fragmentProp, shader);
			}
			if (geometryProp.isValid())	{
				loadShaderSource(geometryProp, shader);
			}
		}

		return shader;
	}
		
	void GL2ShaderManager::onResourceLoaded(ResourceLoader::DataRequest& dr)
	{
		std::string sourceType = dr.meta->properties["source_type"].getValueString();

		GL2Shader* shader = sourceShaderRelation[dr.meta];

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

		resourceLoaded.push(shader->getManagerResourceId());
	}

	void GL2ShaderManager::onResourceLoadFail(ResourceLoader::DataRequest& dr)
	{

	}
}