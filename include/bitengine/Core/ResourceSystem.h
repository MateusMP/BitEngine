#pragma once

#include <string>
#include <type_traits>
#include <vector>

#include "Core/System.h"
#include "Core/SpriteManager.h"
#include "Core/ModelManager.h"
#include "Core/Resources/ResourceManager.h"

namespace BitEngine {

	class ResourceSystem : public System
	{
		public:
			ResourceSystem(ResourceLoader* loader);
			~ResourceSystem();

			bool Init() override;
			void Update() override;
			void Shutdown() override;

			ResourceLoader* getResourceLoader() const;

		private:
			ResourceLoader* loader;
	};

}
