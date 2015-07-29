#pragma once

#include <unordered_map>

#include "Sprite2DShader.h"
#include "EntitySystem.h"

#include "Sprite2DComponent.h"
#include "Camera2DProcessor.h"


namespace BitEngine{

class Sprite2DRenderer{
public:
	Sprite2DRenderer(const EntitySystem* _es);

	bool Init();
	
	Sprite2DShader* getShader();

	void clearRenderer();

	void addToRender(const Sprite2DComponent* sprite, const Transform2DComponent* transform);

	void Render();

private:
	class BatchRenderer
	{
	public:
		BatchRenderer(SpriteManager* sprMng, Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS]);
		~BatchRenderer();

		void begin();

		void addComponent(const Transform2DComponent *t, const Sprite2DComponent* c);

		void render();

	public:
		struct Batch{
			Batch(uint32 _offset, int _nI, uint32 _texture, bool tr)
				: offset(_offset), nItems(_nI), texture(_texture), transparent(tr)
			{}

			uint32 offset;
			int nItems;
			uint32 texture;
			bool transparent;
		};

		struct RenderingComponent{
			RenderingComponent(const Transform2DComponent* t, const Sprite2DComponent* s)
				: transform(t), spr(s) {}
			const Transform2DComponent* transform;
			const Sprite2DComponent* spr;
		};

	private:

		void sortComponents();
		void createRenderers();
		void renderBatches();

		static bool compare_DepthTexture(const RenderingComponent& a, const RenderingComponent& b);

	private:
		SpriteManager* m_spriteManager;

		Sprite2DComponent::SORT_TYPE m_sorting;

		GLuint m_vao;
		GLuint m_vbo[Sprite2DShader::NUM_VBOS];

		std::vector<RenderingComponent> m_components; // Components used by this batch - sorted

		// Batches
		std::vector<Batch> batches;
	};

private:
	void createRenderers();

	std::vector<BatchRenderer*> m_batchRenderers; // Sorted by draw order

	Sprite2DShader *shader;
	const EntitySystem *m_es;

	SpriteManager* m_spriteManager;
};

class Sprite2DProcessor : public ComponentHolderProcessor
{
	public:
		Sprite2DProcessor();
		~Sprite2DProcessor();
		
	private:
		bool Init() override;
		void FrameStart() override {};
		void FrameMiddle() override {};
		void FrameEnd() override;

		ComponentHandle CreateComponent(EntityHandle entity) override;
		void DestroyComponent(ComponentHandle component) override;

		Component* getComponent(ComponentHandle hdl) override;

		const std::vector<ComponentHandle>& getComponents() const override;

	private:
		/*
		* @param screen Screen expected to be a 2D box with bottom left coordinates to top right coordinates
		* @param t Transform2D to be used on calculation
		* @param s Sprite2D to be used on calculation
		*/
		bool insideScreen(const glm::vec4& screen, const Transform2DComponent* t, const Sprite2DComponent* s);

	private:
		ComponentCollection<Sprite2DComponent> components;
};

}