#pragma once

#include <array>

#include "bitengine/Common/ThreadSafeQueue.h"

#include "OpenGL2.h"

#include "bitengine/DefaultBackends/opengl/GL2/GL2Shader.h"

namespace BitEngine {

	class GL2ShaderManager : public ResourceManager
	{
		public:
		GL2ShaderManager();
		~GL2ShaderManager();

		// Resource Manager
		bool init() override;

		void update() override;

		void setResourceLoader(ResourceLoader* loader) override {
			this->loader = loader;
		}

		BaseResource* loadResource(ResourceMeta* base) override;

		void resourceNotInUse(ResourceMeta* meta) override {}
		void reloadResource(BaseResource* resource) override;
		void resourceRelease(ResourceMeta* meta) override {}

		// Shader Manager
		//void Update() override;

		// in bytes
		virtual u32 getCurrentRamUsage() const override {
			return ramInUse;
		}
		virtual u32 getCurrentGPUMemoryUsage() const override {
			return gpuMemInUse;
		}

		void sendToGPU(GL2Shader* shader);

		private:
			ResourceLoader::RawResourceTask loadShaderSource(ResourcePropertyContainer& rpc, GL2Shader* shader);
			void makeFullLoad(ResourceMeta* meta, GL2Shader* shader);

		private:
			ResourceLoader* loader;
			ResourceIndexer<GL2Shader, 32> shaders;
			BitEngine::ThreadSafeQueue<GL2Shader*> resourceLoaded;

			std::unordered_map<ResourceMeta*, GL2Shader*> sourceShaderRelation;

			// Basic shader def
			ShaderDataDefinition basicShaderDef;

			u32 ramInUse;
			u32 gpuMemInUse;
	};

}
