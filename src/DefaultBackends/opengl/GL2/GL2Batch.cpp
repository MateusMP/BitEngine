#include "DefaultBackends/opengl/GL2/GL2Shader.h"
#include "DefaultBackends/opengl/GL2/GL2Pipeline.h"

namespace BitEngine
{
	GL2Batch::GL2Batch(VAOContainer&& vC, const UniformHolder& uC)
		: vaoContainer(vC), uniformContainer(uC)
	{
		// Setup vbo data buffers
		for (VBOContainer& vbc : vaoContainer.vbos)
		{
			shaderData.emplace(vbc.ref, ShaderData(&vbc, 32 * vbc.stride));
		}
		
		IncludeToMap(shaderData, uniformContainer, 0);

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
			glDeleteBuffers(1, &container.vbo);
		}
		GL2::deleteVaos(1, &vaoContainer.vao);
	}

	// Prepare the batch to be rendered
	// Usefull when rendering multiple times the same batch
	void GL2Batch::prepare(u32 numInstances)
	{
		for (VBOContainer& vbc : vaoContainer.vbos)
		{
			shaderData.at(vbc.ref).data.resize(numInstances * vbc.stride);
		}
	}

	IBatchSector* GL2Batch::addSector(u32 begin)
	{
		sectors.emplace_back(uniformContainer, begin);

		GL2BatchSector& bs = sectors[sectors.size() - 1];

		return &bs;
	}

	void GL2Batch::load()
	{
		for (auto& it = shaderData.begin(); it != shaderData.end(); ++it)
		{
			if (it->first.mode == DataUseMode::Vertex)
			{
				GL2::bindVbo(it->second.definition.vbo->vbo);
				GL2::loadBufferRange(it->second.data.data(), 0, it->second.data.size(), GL_STREAM_DRAW);
				GL2::unbindVbo();
			}

			/*float* data = (float*)it->second.data.data();
			printf("Size: %d\n", it->second.data.size() / 4);
			for (int i = 0; i < it->second.data.size() / 4; i += 4) {
				printf("% 5.3f, % 5.3f; %3.2f|%3.2f\n", data[i+0], data[i+1], data[i+2], data[i+3]);
			}
			printf("\n");*/
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
		}
	}

	void GL2Batch::render(Shader* shader)
	{
		GL2Shader *glShader = static_cast<GL2Shader*>(shader);
		glShader->Bind();
		GL2::bindVao(vaoContainer.vao);

		for (auto& it = shaderData.begin(); it != shaderData.end(); ++it)
		{
			if (it->first.mode == DataUseMode::Uniform)
			{
				const char* addr = static_cast<const char*>(it->second.data.data());
				for (const UniformDefinition& def : it->second.definition.unif->defs)
				{
					glShader->loadConfig(&def, (void*)(addr + (u32)def.dataOffset));
				}
			}
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