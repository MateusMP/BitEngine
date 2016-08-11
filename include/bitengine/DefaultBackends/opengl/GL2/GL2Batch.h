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
		GL2BatchSector(const UniformContainer* uc, u32 begin);

		inline void end(u32 finalInstance) override {
			m_end = finalInstance;
		}

		void configure(Shader* shader) override;

		void* getConfigValue(const ShaderDataDefinition::DefinitionReference& ref) override;

		private:
		inline void* getConfigValueForRef(const UniformData& ud) {
			return ud.unif.dataOffset;
		}

		std::unordered_map<ShaderDataDefinition::DefinitionReference, UniformData, ShaderDataDefinition::DefinitionReference::Hasher> configs; // uniforms with instanced = 1
		std::vector<char> data;
		u32 uniformSizeTotal;

		u32 m_begin;
		u32 m_end;
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
		GL2Batch(VAOContainer&& vC, const UniformContainer& uC);
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
		void loadVertexData(const ShaderDataDefinition::DefinitionReference& ref, char *data, u32 nBytes, u32 strideSize) override;

		void* getVertexDataAddress(const ShaderDataDefinition::DefinitionReference& ref, u32 inst) override;

		// Get the uniform config data
		void* getConfigData(const ShaderDataDefinition::DefinitionReference& ref);
		
		private:
		// Bind the buffer to receive data on gpu
		void bindBuffer(AttributeData& container);

		// Unbind the buffer
		void unbindBuffer();

		// Load data to the current loaded buffer on gpu
		void loadBufferRange(const char* data, u32 offset, u32 len, u32 mode = GL_DYNAMIC_DRAW);

		// Load data to the buffer on gpu
		void loadBuffer(const std::vector<char>& data, u32 mode = GL_DYNAMIC_DRAW);

		// Bind the VAO related with this batch
		void bind();

		// Unbind vao
		void unbind();

		// Members
		GLint renderMode;
		VAOContainer vaoContainer;
		UniformContainer uniformContainer;
		std::vector<GL2BatchSector> sectors;
		std::unordered_map<ShaderDataDefinition::DefinitionReference, AttributeData, ShaderDataDefinition::DefinitionReference::Hasher> attributeData;
		std::unordered_map<ShaderDataDefinition::DefinitionReference, UniformData, ShaderDataDefinition::DefinitionReference::Hasher> uniformConfigs; // only uniforms with instance = 0
		std::vector<char> uniformData;
	};
}