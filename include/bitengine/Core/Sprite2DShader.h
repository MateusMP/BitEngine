#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Core/Sprite.h"
#include "Core/Graphics.h"

namespace BitEngine
{
	/// Draws 2D sprites using 2D coordinates
	
	class Sprite2DShader
	{
		public:
			static const u32 TEXTURE_DIFFUSE = 0;
			
			static RendererVersion GetRendererVersion(){ return useRenderer; }

			// Initialize shader for current best available GL version
			// Forces to given version (if not possible, will fallback to an older one)
			// This enables that newer computers execute the code path for older GL (testing purpouses)
			static Sprite2DShader* CreateShader(RendererVersion forceVersion = NOT_DEFINED);

			class Sprite2DBatchRenderer
			{
				public:
					virtual ~Sprite2DBatchRenderer(){}

					virtual void Begin() = 0;
					virtual void End() = 0;

					virtual void setSorting(bool sort) = 0;

					virtual void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth) = 0;

					virtual void Render() = 0;

					struct RenderingElement {
						RenderingElement(u32 _depth, const Sprite* _sprite, const glm::mat3* _modelMatrix)
							: depth(_depth), sprite(_sprite), modelMatrix(_modelMatrix)
						{}

						u32 depth;
						const Sprite* sprite;
						const glm::mat3* modelMatrix;
					};

					static bool compare_DepthTexture(const RenderingElement& a, const RenderingElement& b){
						return a.depth < b.depth
							|| (a.depth == b.depth && (a.sprite->getTexture() < b.sprite->getTexture()));
					}
			};

			///
			///

			Sprite2DShader();
			virtual ~Sprite2DShader(){}

			virtual int Init() = 0;
			
			virtual void BindShader() = 0;

			virtual void LoadViewMatrix(const glm::mat4& matrix) = 0;

			virtual Sprite2DBatchRenderer* CreateRenderer() const = 0;

		private:
			static RendererVersion useRenderer;

	};


}