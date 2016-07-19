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
			void onResourceLoaded(uint32 resourceID) override;

			void onResourceLoadFail(uint32 resourceID) override;


			// Always returns a valid pointer.
			// The shader may be invalid until it is fully loaded
			IShader* getShader(const std::string& str);

			// by resource id
			IShader* getShader(uint32 id);

		private:
			ResourceLoader* loader;
			ResourceIndexer<GL2Shader, 32> shaders;
			BitEngine::ThreadSafeQueue<uint16> resourceLoaded;
			std::unordered_map<uint32, uint16> loadRequests;

			// Basic shader def
			ShaderDataDefinition basicShaderDef;

			uint32 ramInUse;
			uint32 gpuMemInUse;
	};

}