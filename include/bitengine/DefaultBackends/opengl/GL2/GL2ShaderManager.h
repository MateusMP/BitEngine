#pragma once

#include <array>

#include "Common/ThreadSafeQueue.h"

#include "OpenGL2.h"

namespace BitEngine {

	class GL2ShaderManager : public ResourceManager
	{
		public:
		GL2ShaderManager();
		~GL2ShaderManager() {};

		// Resource Manager
		bool init() override;

		void update() override;

		void setResourceLoader(ResourceLoader* loader) override {
			this->loader = loader;
		}

		BaseResource* loadResource(ResourceMeta* base) override;

		// Shader Manager
		//void Update() override;

		// in bytes
		virtual uint32 getCurrentRamUsage() const override {
			return ramInUse;
		}
		virtual uint32 getCurrentGPUMemoryUsage() const override {
			return gpuMemInUse;
		}

		private:
			void onResourceLoaded(ResourceLoader::DataRequest& dr) override;
			void onResourceLoadFail(ResourceLoader::DataRequest& dr) override;

			void loadShaderSource(ResourcePropertyContainer& rpc, GL2Shader* shader);

		private:
			ResourceLoader* loader;
			ResourceIndexer<GL2Shader, 32> shaders;
			BitEngine::ThreadSafeQueue<uint32> resourceLoaded;

			std::unordered_map<ResourceMeta*, GL2Shader*> sourceShaderRelation;

			// Basic shader def
			ShaderDataDefinition basicShaderDef;

			uint32 ramInUse;
			uint32 gpuMemInUse;
	};

}