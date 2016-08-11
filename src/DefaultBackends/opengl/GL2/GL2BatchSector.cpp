#include "DefaultBackends/opengl/GL2/GL2Batch.h"
#include "DefaultBackends/opengl/GL2/GL2Shader.h"

namespace BitEngine {

	GL2BatchSector::GL2BatchSector(const UniformContainer* uc, u32 begin)
		: m_begin(begin), m_end(0), uniformSizeTotal(0)
	{
		uniformSizeTotal = uc->calculateMaxDataSize(1);
		data.resize(uniformSizeTotal);

		for (auto& it = uc->begin(); it != uc->end(); ++it)
		{
			if (it->instanced == 1) {
				char* dataAddr = data.data() + (size_t)(it->dataOffset);
				configs.emplace(it->ref, UniformData(*it, dataAddr));
			}
		}
	}

	void GL2BatchSector::configure(Shader* shader)
	{
		GL2Shader* glShader = static_cast<GL2Shader*>(shader);

		if (!glShader->isReady()) {
			LOG(EngineLog, BE_LOG_WARNING) << "Shader is not ready yet";
			return;
		}

		for (auto it = configs.begin(); it != configs.end(); ++it)
		{
			glShader->loadConfig(it->second.unif.ref, it->second.unif.dataOffset);
		}
	}

	void* GL2BatchSector::getConfigValue(const ShaderDataDefinition::DefinitionReference& ref)
	{
		auto it = configs.find(ref);
		if (it != configs.end())
		{
			return getConfigValueForRef(it->second);
		}

		return nullptr;
	}

}