#include "DefaultBackends/opengl/GL2/GL2Batch.h"
#include "DefaultBackends/opengl/GL2/GL2Impl.h"

#include "DefaultBackends/opengl/GL2/GL2Shader.h"

namespace BitEngine
{
	static void includeToMap(ShaderDataMap& map, const UniformHolder& holder, u32 instance)
	{
		for (const UniformContainer& it : holder.containers) {
			map.emplace(it.ref, ShaderData(&it, it.stride));
		}
	}

	GL2Batch::GL2Batch(VAOContainer&& vC, const UniformHolder& uC)
		: vaoContainer(vC), uniformContainer(uC)
	{
		// Setup vbo data buffers
		for (VBOContainer& vbc : vaoContainer.vbos) {
			shaderData.emplace(vbc.ref, ShaderData(&vbc, 32 * vbc.stride));
		}
		
		includeToMap(shaderData, uniformContainer, 0);

		renderMode = GL_TRIANGLE_STRIP;
	}

	GL2Batch::~GL2Batch()
	{
		clearVAO();
	}

	void GL2Batch::clearVAO()
	{
		for (VBOContainer& container : vaoContainer.vbos)
		{
			GL_CHECK(glDeleteBuffers(1, &container.vbo));
		}
		GL2::deleteVaos(1, &vaoContainer.vao);
	}

	// Prepare the batch to be rendered
	// Usefull when rendering multiple times the same batch
	void GL2Batch::prepare(u32 numInstances)
	{
		totalInstances = numInstances;
		for (VBOContainer& vbc : vaoContainer.vbos) {
			shaderData.at(vbc.ref).data.resize(totalInstances * vbc.stride);
		}
	}

	void GL2Batch::load()
	{
		for (const auto& it : shaderData)
		{
			if (it.first.mode == DataUseMode::Vertex)
			{
				GL2::bindVbo(it.second.definition.vbo->vbo);
				GL2::loadBufferRange(it.second.data.data(), 0, it.second.data.size(), GL_STREAM_DRAW);
				GL2::unbindVbo();
			}
		}
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
				break;
		}
	}

	void GL2Batch::render(Shader* shader)
	{
		GL2Shader *glShader = static_cast<GL2Shader*>(shader);
		glShader->bind();

		for (const auto& it : shaderData)
		{
			if (it.first.mode == DataUseMode::Uniform)
			{
				const char* addr = static_cast<const char*>(it.second.data.data());
				for (const UniformDefinition* def : it.second.definition.unif->defs)
				{
					glShader->loadConfig(def, static_cast<const void*>(addr + def->dataOffset));
				}
			}
		}

		GL2::bindVao(vaoContainer.vao);
		//bs.configure(shader);
		/*if (r.transparent)
		{
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}*/
		// glBindTexture(GL_TEXTURE_2D, r.texture);

		GL_CHECK(glDrawArrays(renderMode, 0, totalInstances));
		//GL_CHECK(glDrawArraysInstancedBaseInstance(renderMode, 0, 4, nItems, bs.m_begin));

		/*if (r.transparent) {
		glDisable(GL_BLEND);
		}*/
		GL2::unbindVao();

		glShader->Unbind();
	}


	void* GL2Batch::getShaderData(const ShaderDataReference& ref)
	{
		auto it = shaderData.find(ref);
		if (it == shaderData.end())
		{
			// [container].strideSize != strideSize
			LOG(EngineLog, BE_LOG_ERROR) << "Invalid ref or stride size!";
			throw "Invalid Shader Data Reference";
		}
		else
		{
			return it->second.data.data();
		}
	}
}
