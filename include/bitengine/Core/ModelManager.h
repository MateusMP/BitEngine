#pragma once

#include <string>
#include <vector>
#include <map>

#include "DefaultBackends/opengl/OpenGL.h"

#include <glm/common.hpp>

#include "Common/TypeDefinition.h"

#include "Core/Logger.h"
#include "Core/Mesh.h"
#include "Core/Resources/ResourceManager.h"

namespace BitEngine{

	class ModelManager
	{
	public:
		ModelManager(ResourceLoader* textureMng) {
			m_textureManager = textureMng;
		}
		~ModelManager();

		bool Init();

		/**
		 * Load a model for use in a given shader.
		 * Since each shader uses different data, the function will
		 * call a static method from the ShaderClass passed on template
		 * called LoadModel(TextureManager* tmng, std::string& filename).
		 * Any textures that need to be loaded should be load be calling the
		 * corresponding method on the TextureManager.
		 * This method should return a pointer to a subclass of BitEngine::Model,
		 * that will be stored inside the ModelManager.
		 * Posterior calls for loading the same filename with the same shader will
		 * return the already loaded model.
		 *
		 * The ShaderClass should also have another static method: int GetID()
		 * that should return an unique int for identifying the shader type.
		 */
		template<typename ShaderClass>
		typename ShaderClass::Model* loadModel(const std::string& filename)
		{
			ShaderModelHolder& holder = m_models[ShaderClass::GetID()];
			typename ShaderClass::Model* model = static_cast<typename ShaderClass::Model*>(holder.getModel(filename));

			if (model == nullptr)
			{
				model = ShaderClass::LoadModel(m_textureManager, filename);

				if (model != nullptr){
					holder.modelsByName.insert(std::pair<std::string, Model*>(filename, model));
				} else {
					LOG(EngineLog, BE_LOG_ERROR) << "Failed to load model from file " << filename;
				}
			}

			return model;
		}

		template<typename ShaderClass>
		typename ShaderClass::Model* loadModelFromMemory(const void* memory, size_t size, const std::string& name){
			ShaderModelHolder& holder = m_models[ShaderClass::GetID()];
			Model* model = holder.getModel(name);

			if (model == nullptr)
			{
				model = ShaderClass::LoadModel(m_textureManager, memory, size);

				if (model != nullptr){
					holder.modelsByName.insert(std::pair<std::string, Model*>(name, model));
					// holder.models.push_back(model);
				}
				else {
					LOG(EngineLog, BE_LOG_ERROR) << "Failed to load model from file " << name;
				}
			}

			return (typename ShaderClass::Model*) model;
		}

	private:
		struct ShaderModelHolder{
			Model* getModel(const std::string& name){
				auto it = modelsByName.find(name);
				if (it == modelsByName.end())
					return nullptr;
				return it->second;
			}

			std::map<std::string, Model*> modelsByName;
		};

		std::map<u16, ShaderModelHolder> m_models;
		ResourceLoader* m_textureManager;
	};
}
