#pragma once

#include <unordered_map>

#include "OpenGL2.h"

#include "GL2Structs.h"

namespace BitEngine
{
	struct ShaderData
	{
		ShaderData(const UniformContainer* unif, u32 dataSize)
		{
			definition.unif = unif;
			data.resize(dataSize);
		}

		ShaderData(const VBOContainer* vbo, u32 dataSize)
		{
			definition.vbo = vbo;
			data.resize(dataSize);
		}

		~ShaderData()
		{
		}
		
		union {
			const UniformContainer* unif;
			const VBOContainer* vbo;
		} definition;

		std::vector<char> data;
	};

	typedef std::unordered_map<ShaderDataReference, ShaderData, ShaderDataReference::Hasher> ShaderDataMap;

	static void IncludeToMap(ShaderDataMap& map, const UniformHolder& holder, u32 instance)
	{
		for (auto& it = holder.containers.begin(); it != holder.containers.end(); ++it)
		{
			if (it->instance == instance)
			{
				map.emplace(it->ref, ShaderData(&(*it), it->stride));
			}
		}
	}

	class GL2BatchSector : public IBatchSector
	{
		friend class GL2Batch;
		public:
			GL2BatchSector(const UniformHolder& uc, u32 begin);

			inline void end(u32 finalInstance) override {
				m_end = finalInstance;
			}

			void configure(Shader* shader) override;

			void* getShaderData(const ShaderDataReference& ref) override;

		private:
			ShaderDataMap shaderData; // uniforms with instanced = 1

			u32 m_begin;
			u32 m_end;
	};

	// GL BATCH
	class GL2Batch : public IGraphicBatch
	{
	public:
		GL2Batch(VAOContainer&& vC, const UniformHolder& uC);
		~GL2Batch();

		// Deleta VAO/VBO from gpu.
		void clearVAO();

		// Clear all sectors.
		void clear() override
		{
			sectors.clear();
		}

		// Prepare the batch to be rendered
		// Usefull when rendering multiple times the same batch
		void prepare(u32 numInstances) override;

		// Add a new sector
		IBatchSector* addSector(u32 begin) override;

		// Load it to the gpu
		void load() override;

		// Render the loaded data
		void render(Shader* shader) override;

		void setVertexRenderMode(VertexRenderMode mode) override;

	protected:
		// Load data into internal buffer
		// This will replace all data from the internal buffer
		void* getShaderData(const ShaderDataReference& ref) override;

	private:
		// Members
		GLint renderMode;
		VAOContainer vaoContainer;
		const UniformHolder& uniformContainer;
		std::vector<GL2BatchSector> sectors;
		ShaderDataMap shaderData;
	};
}