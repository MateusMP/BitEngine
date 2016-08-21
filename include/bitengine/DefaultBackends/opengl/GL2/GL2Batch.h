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
    typedef std::unordered_map<ShaderDataReference, ShaderData, ShaderDataReference::Hasher>::iterator ShaderDataMapIt;

	// GL BATCH
	class GL2Batch : public IGraphicBatch
	{
	public:
		GL2Batch(VAOContainer&& vC, const UniformHolder& uC);
		~GL2Batch();

		// Delete VAO/VBO from gpu.
		void clearVAO();

		// Clear all sectors.
		void clear() override
		{
			totalInstances = 0;
		}

		// Prepare the batch to be rendered
		// Usefull when rendering multiple times the same batch
		void prepare(u32 numInstances) override;

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
		u32 totalInstances;
		ShaderDataMap shaderData;
	};
}
