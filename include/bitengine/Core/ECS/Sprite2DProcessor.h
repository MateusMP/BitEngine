#pragma once

#include <unordered_set>

#include "Core/ECS/Sprite2DComponent.h"
#include "Core/ECS/Camera2DProcessor.h"

// processor
#include "Core/Sprite2DShader.h"

#include "Common/ErrorCodes.h"

namespace BitEngine{

class Sprite2DRendererBasic : public ComponentProcessor
{
	public:
		Sprite2DRendererBasic(Transform2DProcessor *t2dp, ResourceLoader* sprManager)
			: ComponentProcessor(t2dp->getMessenger()), transform2DProcessor(t2dp)
		{
			m_spriteManager = sprManager;
		}

		// Processor
		bool Init() override
		{
			holderCamera = getES()->getHolder<Camera2DComponent>();
			if (holderCamera == nullptr)
				return false;

			spr2DShader = BitEngine::Sprite2DShader::CreateShader();
			if (spr2DShader->Init() == BE_NO_ERROR)
			{
				spr2Dbatch = spr2DShader->CreateRenderer();
				if (spr2Dbatch == nullptr) {
					LOG(EngineLog, BE_LOG_ERROR) << "Could not create Sprite2D renderer";
					return false;
				}
			}
			else
			{
				return false;
			}

			return true;
		}

		void Stop() override
		{
			delete spr2Dbatch;
			delete spr2DShader;
		}

		void setActiveCamera(ComponentRef<Camera2DComponent>& handle)
		{
			activeCamera = handle;
		}

		void Render()
		{
			if (!activeCamera.isValid())
				return;

			m_viewScreen = activeCamera->getWorldViewArea();

			spr2Dbatch->Begin();
			/*
			getES()->forEachRef<Sprite2DRendererBasic, Sprite2DComponent, SceneTransform2DComponent>(
				[](Sprite2DRendererBasic& self, EntityHandle entity, Sprite2DComponent& spr2d, ComponentRef<Transform2DComponent>&& transform2d)
				{
					//printf("Sprite %d\n", i);
					const glm::mat3& transform = self.transform2DProcessor->getGlobalTransformFor(getComponentHandle(transform2d));
					const Sprite* sprite = self.m_spriteManager->getSprite(spr2d.sprite);

					if (insideScreen(self.m_viewScreen, transform, sprite->getMaxRadius()))
					{
						self.spr2Dbatch->DrawSprite(sprite,
							&transform,
							spr2d.depth);
					}
				}
			);*/

			spr2Dbatch->End();

			// Set up shader
			spr2DShader->LoadViewMatrix(activeCamera->getMatrix());
			spr2DShader->BindShader();

			// Set up GL states
			glEnable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);

			// Render
			spr2Dbatch->Render();

			glDisable(GL_CULL_FACE);
		}

		static bool insideScreen(const glm::vec4& screen, const glm::mat3& matrix, float radius)
		{
			const float kX = matrix[2][0] + radius;
			const float kX_r = matrix[2][0] - radius;
			const float kY = matrix[2][1] + radius;
			const float kY_b = matrix[2][1] - radius;

			if (kX < screen.x) {
				// printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE left %p - %f | %f\n", t, kX, screen.x);
				return false;
			}
			if (kX_r > screen.z) {
				//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE right %p - %f | %f\n", t, kX_r, screen.z);
				return false;
			}
			if (kY < screen.y) {
				//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE bot %p - %f | %f\n", t, kY, screen.y);
				return false;
			}
			if (kY_b > screen.w) {
				//printf(">>>>>>>>>>>>>>>>>>>>>>> HIDE top %p - %f | %f\n", t, kY_b, screen.w);
				return false;
			}

			return true;
		}

	private:
		struct Entry {
			Entry(EntityHandle ent, ComponentHandle spr, ComponentHandle transform)
				: entity(ent), sprite2d(spr), transform2d(transform) {}

			EntityHandle entity;
			ComponentHandle sprite2d;
			ComponentHandle transform2d;
		};

		ComponentRef<Camera2DComponent> activeCamera;

		ComponentHolder<Camera2DComponent> *holderCamera;
		Transform2DProcessor *transform2DProcessor;

		glm::vec4 m_viewScreen;

		const BitEngine::ResourceLoader *m_spriteManager;
		BitEngine::Sprite2DShader *spr2DShader;
		BitEngine::Sprite2DShader::Sprite2DBatchRenderer *spr2Dbatch;
};


}
