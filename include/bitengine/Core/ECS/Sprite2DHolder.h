#pragma once

#include <unordered_set>

#include "Core/ECS/Sprite2DComponent.h"
#include "Core/ECS/Camera2DProcessor.h"

// processor
#include "Core/Sprite2DShader.h"

#include "Common/ErrorCodes.h"

namespace BitEngine{

class Sprite2DHolder : public ComponentHolder
{
	public:
		Sprite2DHolder();
		~Sprite2DHolder();

		// Holder
		Component* getComponent(ComponentHandle hdl) override;

	private: // Functions
		// Holder
		ComponentHandle AllocComponent() override;
		void DeallocComponent(ComponentHandle component) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private: // members
		// Holder
		ComponentCollection<Sprite2DComponent> components;
};

class Sprite2DRendererBasic : public ComponentProcessor
{
	public:
		Sprite2DRendererBasic(const SpriteManager* sprManager) 
			: renderCamera(0)
		{
			m_spriteManager = sprManager;
		}

		// Processor
		bool Init(BaseEntitySystem* es) override 
		{
			sprite2DType = es->getComponentType<Sprite2DComponent>();
			transform2DType = es->getComponentType<Transform2DComponent>();
			camera2DType = es->getComponentType<Camera2DComponent>();
	
			holderSprite = static_cast<Sprite2DHolder*>(es->getHolder(sprite2DType));
			holderTransform = static_cast<Transform2DProcessor*>(es->getHolder(transform2DType));
			holderCamera = static_cast<Camera2DProcessor*>(es->getHolder(camera2DType));
			if (holderSprite == nullptr || holderTransform == nullptr || holderCamera == nullptr)
				return false;

			holderSprite->RegisterListener(this);

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

		void Stop() override {
			delete spr2Dbatch;
			delete spr2DShader;

			holderSprite->UnregisterListener(this);
		}

		void setActiveCamera(ComponentHandle handle)
		{
			renderCamera = handle;
		}

		void Render()
		{
			if (renderCamera == 0)
				return;

			Camera2DComponent* activeCamera = static_cast<Camera2DComponent*>(holderCamera->getComponent(renderCamera));
			const glm::vec4& viewScreen = activeCamera->getWorldViewArea();

			spr2Dbatch->Begin();

			for (const Entry& e : processEntries)
			{
				//printf("Sprite %d\n", i);
				const Sprite2DComponent* spr = static_cast<Sprite2DComponent*>( holderSprite->getComponent(e.sprite2d) );
				const glm::mat3& transform = holderTransform->getGlobalTransformFor(e.transform2d);
				const Sprite* sprite = m_spriteManager->getSprite(spr->sprite);

				if (insideScreen(viewScreen, transform, sprite->getMaxRadius()))
				{
					spr2Dbatch->DrawSprite( sprite, 
											&transform,
											spr->depth);
				}
			}

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

		void OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component) override {
			ComponentHandle sprite = holderSprite->getComponentHandleFor(entity);
			ComponentHandle transform = holderTransform->getComponentHandleFor(entity);

			if (sprite && transform) {
				processEntries.emplace_back(entity, sprite, transform);
			}
		}

		void OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component) override {
			invalidatedEntries.emplace(entity);
		}

		void setRenderCamera(ComponentHandle handle) {
			renderCamera = handle;
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

		ComponentType sprite2DType;
		ComponentType transform2DType;
		ComponentType camera2DType;

		ComponentHandle renderCamera;

		Sprite2DHolder *holderSprite;
		Transform2DProcessor *holderTransform;
		Camera2DProcessor *holderCamera;

		std::vector<Entry> processEntries;
		std::unordered_set<EntityHandle> invalidatedEntries;


		const BitEngine::SpriteManager *m_spriteManager;
		BitEngine::Sprite2DShader *spr2DShader;
		BitEngine::Sprite2DShader::Sprite2DBatchRenderer *spr2Dbatch;
};


}