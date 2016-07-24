#include "DefaultBackends/opengl/GL2/GL2Shader.h"
#include "DefaultBackends/opengl/GL2/GL2Pipeline.h"

namespace BitEngine
{
	GL2Batch::GL2Batch(const VAOContainer& vC, const UniformContainer& uC)
		: vaoContainer(vC), uniformContainer(uC)
	{
		// Setup vbo data buffers
		for (VBOContainer& vbc : vaoContainer.vbos)
		{
			attributeData.emplace(vbc.ref, &vbc);
		}

		uint32 fullUniformSize = 0;
		for (auto& it = uC.begin(); it != uC.end(); ++it)
		{
			if (it->instanced == 0)
			{
				fullUniformSize += it->byteSize;
				uniformConfigs.emplace(it->ref, UniformData(*it, uniformData.data() + (size_t)(it->dataOffset)));
			}
		}
		uniformData.resize(fullUniformSize);

		for (auto& it = uniformConfigs.begin(); it != uniformConfigs.end(); ++it)
		{
			it->second.unif.dataOffset = uniformData.data() + (uint32)(it->second.unif.dataOffset);
		}
	}

	// Prepare the batch to be rendered
	// Usefull when rendering multiple times the same batch
	void GL2Batch::prepare(uint32 numInstances)
	{
		for (auto it = attributeData.begin(); it != attributeData.end(); ++it)
		{
			it->second.data.resize(it->second.vbo->stride * numInstances);
		}
	}

	IBatchSector* GL2Batch::addSector(uint32 begin)
	{
		sectors.emplace_back(&uniformContainer, begin);

		GL2BatchSector& bs = sectors[sectors.size() - 1];

		return &bs;
	}

	void GL2Batch::load()
	{
		for (auto it = attributeData.begin(); it != attributeData.end(); ++it)
		{
			bindBuffer(it->second);
			loadBuffer(it->second.data);
		}

		unbindBuffer();
	}

	void GL2Batch::bindBuffer(AttributeData& container)
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, container.vbo->vbo));
	}

	void GL2Batch::unbindBuffer()
	{
		GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
	}

	void GL2Batch::loadBufferRange(const char* data, uint32 offset, uint32 len, uint32 mode)
	{
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, nullptr, mode));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, data + offset, mode));
	}

	void GL2Batch::loadBuffer(const std::vector<char>& data, uint32 mode)
	{
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, data.size(), nullptr, mode));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, data.size(), data.data(), mode));
	}

	void GL2Batch::bind()
	{
		GL_CHECK(glBindVertexArray(vaoContainer.vao));
	}

	void GL2Batch::unbind()
	{
		GL_CHECK(glBindVertexArray(0));
	}

	void GL2Batch::render(Shader* shader)
	{
		GL2Shader *glShader = static_cast<GL2Shader*>(shader);
		bind();

		for (auto& it = uniformConfigs.begin(); it != uniformConfigs.end(); ++it)
		{
			glShader->loadConfig(it->second.unif.ref, uniformData.data() + (uint32)it->second.unif.dataOffset);
		}

		for (GL2BatchSector& bs : sectors)
		{
			bs.configure(shader);
			/*if (r.transparent)
			{
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}*/
			// glBindTexture(GL_TEXTURE_2D, r.texture);

			const GLsizei nItems = bs.m_end - bs.m_begin;
			GL_CHECK(glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, nItems, bs.m_begin));

			/*if (r.transparent) {
			glDisable(GL_BLEND);
			}*/
		}

		unbind();
	}


	void GL2Batch::loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, char *data, uint32 nBytes, uint32 strideSize)
	{
		auto it = attributeData.find(ref);
		if (it == attributeData.end() || it->second.vbo->stride != strideSize)
		{
			// [container].strideSize != strideSize
			LOG(EngineLog, BE_LOG_ERROR) << "Invalid ref or stride size!";
			return;
		}

		it->second.data.resize(nBytes);
		memcpy(&it->second.data[0], data, nBytes);
	}

	void* GL2Batch::getVertexDataAddress(const ShaderDataDefinition::DefinitionReference& ref, uint32 inst)
	{
		auto it = attributeData.find(ref);
		if (it == attributeData.end())
		{
			LOG(EngineLog, BE_LOG_ERROR) << "Invalid ref or stride size!";
			return nullptr;
		}

		return it->second.data.data() + it->second.vbo->stride * inst;
	}

	void* GL2Batch::getConfigData(const ShaderDataDefinition::DefinitionReference& ref)
	{
		auto it = uniformConfigs.find(ref);
		if (it == uniformConfigs.end())
		{
			LOG(EngineLog, BE_LOG_ERROR) << "Invalid ref or stride size!";
			return nullptr;
		}

		return uniformData.data() + (uint32)it->second.unif.dataOffset;
	}
}