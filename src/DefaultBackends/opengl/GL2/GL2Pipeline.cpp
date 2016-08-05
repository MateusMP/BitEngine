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

		u32 fullUniformSize = 0;
		for (auto& it = uC.begin(); it != uC.end(); ++it)
		{
			if (it->instanced == 0)
			{
				fullUniformSize += it->byteSize;
				uniformConfigs.emplace(it->ref, UniformData(*it, (char*)(it->dataOffset)));
			}
		}
		uniformData.resize(fullUniformSize);

		for (auto& it = uniformConfigs.begin(); it != uniformConfigs.end(); ++it)
		{
			it->second.unif.dataOffset = uniformData.data() + (u32)(it->second.unif.dataOffset);
		}

		renderMode = GL_TRIANGLE_STRIP;
	}

	// Prepare the batch to be rendered
	// Usefull when rendering multiple times the same batch
	void GL2Batch::prepare(u32 numInstances)
	{
		for (auto it = attributeData.begin(); it != attributeData.end(); ++it)
		{
			it->second.data.resize(it->second.vbo->stride * numInstances);
		}
	}

	IBatchSector* GL2Batch::addSector(u32 begin)
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

	void GL2Batch::loadBufferRange(const char* data, u32 offset, u32 len, u32 mode)
	{
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, nullptr, mode));
		GL_CHECK(glBufferData(GL_ARRAY_BUFFER, len, data + offset, mode));
	}

	void GL2Batch::loadBuffer(const std::vector<char>& data, u32 mode)
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

	void GL2Batch::setVertexRenderMode(VertexRenderMode mode)
	{
		switch (mode)
		{
			case VertexRenderMode::TRIANGLES:
				renderMode = GL_TRIANGLES;
				break;
			case VertexRenderMode::TRIANGLE_STRIP:
				renderMode = GL_TRIANGLE_STRIP;
				break;
			default:
				LOG(EngineLog, BE_LOG_ERROR) << "Invalid render mode " << (int)mode;
		}
	}

	void GL2Batch::render(Shader* shader)
	{
		GL2Shader *glShader = static_cast<GL2Shader*>(shader);
		glShader->Bind();
		bind();

		for (auto& it = uniformConfigs.begin(); it != uniformConfigs.end(); ++it)
		{
			glShader->loadConfig(it->second.unif.ref, uniformData.data() + (u32)it->second.unif.dataOffset);
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
			GL_CHECK(glDrawArrays(renderMode, bs.m_begin, nItems));
			//GL_CHECK(glDrawArraysInstancedBaseInstance(renderMode, 0, 4, nItems, bs.m_begin));

			/*if (r.transparent) {
			glDisable(GL_BLEND);
			}*/
		}

		unbind();
	}


	void GL2Batch::loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, char *data, u32 nBytes, u32 strideSize)
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

	void* GL2Batch::getVertexDataAddress(const ShaderDataDefinition::DefinitionReference& ref, u32 inst)
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

		return uniformData.data() + (u32)it->second.unif.dataOffset;
	}
}