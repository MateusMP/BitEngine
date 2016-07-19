#include "Common/MathUtils.h"
#include "Core/Logger.h"

#include "DefaultBackends/opengl/GL2/GL2Shader.h"
#include "DefaultBackends/opengl/GL2/GL2ShaderManager.h"


namespace BitEngine {

	//

	GL2ShaderManager::GL2ShaderManager()
	{
		ramInUse = 0;
		gpuMemInUse = 0;
	}

	bool GL2ShaderManager::init()
	{
		// Init error texture
		GL2Shader& basicShader = shaders.getResourceAt(0);
		basicShader.init(basicShaderDef);

		return true;
	}

	void GL2ShaderManager::update()
	{

	}

	/*void GL2ShaderManager::setResourceLoader(BitEngine::ResourceLoader* loader)
	{
		this->loader = loader;
	}*/

	/*void GL2ShaderManager::Update()
	{
		uint16 localID;

		bool loadedSomething = false;
		while (resourceLoaded.tryPop(localID))
		{
			GL2Shader& data = shaders.getResourceAt(localID);
			
			// load texture
			if (data.imgData.pixelData != nullptr)
			{
				loadTexture2D(data.imgData, data);
			}
			else // Use error texture
			{
				data.m_textureID = shaders.getResourceAt(0).m_textureID;
			}

			// Do not need the file data anymore
			data.clearBaseData();

			// delete pixel data too
			if (data.imgData.pixelData != nullptr) {
				stbi_image_free(data.imgData.pixelData);
				data.imgData.pixelData = nullptr;
			}

			const uint32 ram = data.getUsingRamMemory();
			const uint32 gpuMem = data.getUsingGPUMemory();
			ramInUse += ram;
			gpuMemInUse += gpuMem;

			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << data.path << "  RAM: " << BitEngine::BytesToMB(ram) << " MB - GPU Memory: " << BitEngine::BytesToMB(gpuMem) << " MB";

			loadedSomething = true;
			
			break;
		}

		if (loadedSomething) {
			LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "TextureManager MEMORY: RAM: " << BitEngine::BytesToMB(getCurrentRamUsage()) << " MB - GPU Memory: " << BitEngine::BytesToMB(getCurrentGPUMemoryUsage()) << " MB";
		}

	}*/

	BaseResource* GL2ShaderManager::loadResource(ResourceMeta* meta)
	{
		GL2Shader* shader = shaders.findResource(meta);

		if (shader == nullptr)
		{
			uint16 id = shaders.addResource(meta);
			shader = &(shaders.getResourceAt(id));

			// Make new load request
			//uint32 request = loader->loadRequest(str, shader, this);
			//loadRequests[request] = id;
		}

		return shader;
	}
		
	void GL2ShaderManager::onResourceLoaded(uint32 resourceID)
	{
		uint16 localID = loadRequests[resourceID];
		GL2Shader& resTexture = shaders.getResourceAt(localID);

		// Working on ResourceLoader thread!
		{
			LOG_SCOPE_TIME(BitEngine::EngineLog, "Texture load");
/*
			resTexture.imgData.pixelData = stbi_load_from_memory((unsigned char*)resTexture.data.data(), resTexture.data.size(), &resTexture.imgData.width, &resTexture.imgData.height, &resTexture.imgData.color, 0);
			if (resTexture.imgData.pixelData != nullptr) {
				LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "stbi loaded texture: " << resTexture.getPath() << " w: " << resTexture.imgData.width << " h: " << resTexture.imgData.height;
			}
			else {
				LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "stbi failed to load texture: " << resTexture.path;
			}*/
		}

		resourceLoaded.push(localID);
	}

	void GL2ShaderManager::onResourceLoadFail(uint32 resourceID)
	{
		//LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Failed to load " << shaders.getResourceAt(loadRequests[resourceID]).path;
	}
}