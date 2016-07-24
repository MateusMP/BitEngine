#pragma once

#include <unordered_map>

#include "OpenGL2.h"

#include "GL2Structs.h"

namespace BitEngine
{
	class GL2BatchSector : public IBatchSector
	{
		friend class GL2Batch;
		public:
		GL2BatchSector(const UniformContainer* uc, uint32 begin);

		inline void end(uint32 finalInstance) override {
			m_end = finalInstance;
		}

		void configure(Shader* shader) override;

		void* getConfigValue(const ShaderDataDefinition::DefinitionReference& ref) override;

		private:
		inline void* getConfigValueForRef(const UniformData& ud) {
			return (void*)(data.data() + ((size_t)(ud.unif.dataOffset)));
		}

		std::unordered_map<ShaderDataDefinition::DefinitionReference, UniformData, ShaderDataDefinition::DefinitionReference::Hasher> configs; // uniforms with instanced = 1
		std::vector<char> data;
		uint32 uniformSizeTotal;

		uint32 m_begin;
		uint32 m_end;
	};

	// GL BATCH
	class GL2Batch : public IGraphicBatch
	{
		struct AttributeData {
			AttributeData(VBOContainer* c)
				: vbo(c)
			{
			}
			VBOContainer* vbo;
			std::vector<char> data;
		};

		public:
		GL2Batch(const VAOContainer& vC, const UniformContainer& uC);

		// Clear all sectors.
		void clear() override
		{
			sectors.clear();
		}

		// Prepare the batch to be rendered
		// Usefull when rendering multiple times the same batch
		void prepare(uint32 numInstances) override;

		// Add a new sector
		IBatchSector* addSector(uint32 begin) override;

		// Load it to the gpu
		void load() override;

		// Render the loaded data
		void render(Shader* shader) override;

		protected:
		// Load data into internal buffer
		// This will replace all data from the internal buffer
		void loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, char *data, uint32 nBytes, uint32 strideSize) override;

		void* getVertexDataAddress(const ShaderDataDefinition::DefinitionReference& ref, uint32 inst) override;

		// Get the uniform config data
		void* getConfigData(const ShaderDataDefinition::DefinitionReference& ref);

		private:
		// Bind the buffer to receive data on gpu
		void bindBuffer(AttributeData& container);

		// Unbind the buffer
		void unbindBuffer();

		// Load data to the current loaded buffer on gpu
		void loadBufferRange(const char* data, uint32 offset, uint32 len, uint32 mode = GL_DYNAMIC_DRAW);

		// Load data to the buffer on gpu
		void loadBuffer(const std::vector<char>& data, uint32 mode = GL_DYNAMIC_DRAW);

		// Bind the VAO related with this batch
		void bind();

		// Unbind vao
		void unbind();

		// Members
		VAOContainer vaoContainer;
		UniformContainer uniformContainer;
		std::vector<GL2BatchSector> sectors;
		std::unordered_map<ShaderDataDefinition::DefinitionReference, AttributeData, ShaderDataDefinition::DefinitionReference::Hasher> attributeData;
		std::unordered_map<ShaderDataDefinition::DefinitionReference, UniformData, ShaderDataDefinition::DefinitionReference::Hasher> uniformConfigs; // only uniforms with instance = 0
		std::vector<char> uniformData;
	};
}