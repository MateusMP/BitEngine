#pragma once

#include <map>
#include <set>
#include <vector>

#include "Core/Texture.h"

namespace BitEngine{

	template<typename T>
	struct ManagerData
	{
		ManagerData()
			: data(nullptr), countUsing(1), ready(false)
		{}

		ManagerData(T* p) 
			: data(p), countUsing(1), ready(p!=nullptr)
		{}
			
		T* data;
		int countUsing; //!< Current requests for loading this
		bool ready;
	};

	class DataPackageItem
	{
	public:
		DataPackageItem(const std::string& resourcePath) 
			: resourceName(resourcePath)
		{}

	private:
		friend class DataPackage;

		std::string resourceName;
		std::map<std::string, std::string> configs;
	};

	class DataPackage
	{
	public:
		DataPackage()
		{}
		~DataPackage()
		{}

		DataPackageItem* AddItem(const std::string& path){
			m_resources.emplace(path, path);
		}

		const std::map<std::string, DataPackageItem>& getItems() const {
			return m_resources;
		}

	private:
		std::map<std::string, DataPackageItem> m_resources;
	};

	class TextureManager
	{
		public:
			typedef std::map<std::string, ManagerData<Texture>> TextureMap;

			TextureManager();
			~TextureManager();

			bool Init();

			const Texture* getErrorTexture();
			
			/** \brief Returns the reference for given texture name
			 * If the texture isn't already loaded, it will block and load
			 * otherwise the function returns immediately.
			 */
			const Texture* getTexture(const std::string& name);

			/**
			 * Loads the package to memory.
			 * If the resource (Package or item) is already loaded,
			 * the references are counted one more time
			 * to manage multiple packages loading the same resources
			 * and avoiding the release of needed data.
			 */
			void LoadPackage(const DataPackage* package);

			/**
			 * Releases the data used by given package
			 * If more packages asked for loading any of the files
			 * those will only be deleted when all packages
			 * that need them are released.
			 */
			void ReleasePackage(const DataPackage* package);

		private:
			/**
			 * BLOCK UNTIL FINISHES LOADING DATA
			 * Load the texture
			 */
			const Texture* doLoad(const std::string& path);

			/**
			 * Creates a texture 2D with default params
			 */
			Texture* loadTexture2D(const std::string& path);

			/**
			 * Deletes and free's the GL texture object
			 */
			void releasetexture(Texture* texture);

			TextureMap m_textures;
			Texture error_texture;

			std::set<const DataPackage*> m_LoadedPackages;
	};



}