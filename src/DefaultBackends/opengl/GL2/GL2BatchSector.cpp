#include "DefaultBackends/opengl/GL2/GL2Batch.h"
#include "DefaultBackends/opengl/GL2/GL2Shader.h"

namespace BitEngine {

	GL2BatchSector::GL2BatchSector(const UniformHolder& uc, u32 begin)
		: m_begin(begin), m_end(0)
	{
		IncludeToMap(shaderData, uc, 1);
	}

	void GL2BatchSector::configure(Shader* shader)
	{
		GL2Shader* glShader = static_cast<GL2Shader*>(shader);

		if (!glShader->isReady()) {
			LOG(EngineLog, BE_LOG_WARNING) << "Shader is not ready yet";
			return;
		}

		for (const auto& it : shaderData)
		{
			const char* addr = static_cast<const char*>(it.second.data.data());
			for (const UniformDefinition* def : it.second.definition.unif->defs)
			{
				glShader->loadConfig(def, (void*)(addr + def->dataOffset) );
			}
		}
	}

	void* GL2BatchSector::getShaderData(const ShaderDataReference& ref)
	{
		const auto& sd = shaderData.find(ref);
		if (sd != shaderData.end())
		{
			return sd->second.data.data();
		}
		else
		{
			throw "Failed to find shader property";
		}
	}
}