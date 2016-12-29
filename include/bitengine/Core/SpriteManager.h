#pragma once

#include <vector>
#include <map>

#include "bitengine/Core/Resources/ResourceManager.h"
#include "bitengine/Core/Sprite.h"

namespace BitEngine{
	
	class SpriteManager : public  ResourceManager
	{
	public:
		SpriteManager();

		/**
		 * Insert a new sprite
		 * The name is used for name look-up, but all sprites can be found
		 * using the returned SpriteHandle.
		 * \param Name used for indexing
		 * \param Sprite data to be used
		 * \param return SpriteHandle for direct acess to given sprite
		 */
		Sprite* createSprite(const std::string& name, const Sprite& spr);

	private:
		void loadSpriteData(ResourceMeta* meta, Sprite* sprite);
		
		// Inherited via ResourceManager
		virtual bool init() override;

		virtual void update() override;

		void setResourceLoader(ResourceLoader* loader) override {
			resourceLoader = loader;
		}

		virtual BaseResource * loadResource(ResourceMeta* meta) override;
		void resourceNotInUse(ResourceMeta* meta) override {}
		void reloadResource(BaseResource* resource) override;
		void resourceRelease(ResourceMeta* meta) override {}

		virtual u32 getCurrentRamUsage() const override;

		virtual u32 getCurrentGPUMemoryUsage() const override;


		ResourceLoader* resourceLoader;
		ResourceIndexer<Sprite, 2048> sprites;
	};


}