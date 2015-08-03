#include "Sprite2DRenderer.h"

namespace BitEngine{

	Sprite2DRenderer::Sprite2DRenderer()
	{
	}

	bool Sprite2DRenderer::Init()
	{
		shader = new Sprite2DShader();
		if (shader->Init() <= 0)
			return false;

		for (int i = 0; i < (int)SpriteSortType::TOTAL; ++i){
			m_batchRenderers.push_back(new BatchRenderer((SpriteSortType)i));
		}

		return true;
	}

	Sprite2DShader* Sprite2DRenderer::getShader()
	{
		return shader;
	}

	void Sprite2DRenderer::Begin()
	{
		// Clear all batches
		for (BatchRenderer* r : m_batchRenderers){
			r->begin();
		}
	}

	void Sprite2DRenderer::addToRender(SpriteSortType sortmode, const Sprite* sprite, const glm::mat3* modelMatrix, int depth)
	{
		m_batchRenderers[(int)sortmode]->drawSprite(sprite, modelMatrix, depth);
	}

	void Sprite2DRenderer::End(){
		for (BatchRenderer* b : m_batchRenderers)
		{
			b->end();
		}
	}

	void Sprite2DRenderer::Render()
	{
		// Render
		
		shader->Bind();
		for (BatchRenderer* b : m_batchRenderers)
		{
			b->render();
		}
	}

	/// ===============================================================================================
	/// ===============================================================================================
	///										BATCH
	/// ===============================================================================================

	Sprite2DRenderer::BatchRenderer::BatchRenderer(SpriteSortType s)
		: m_sorting(s)
	{
		// Create at least one VAO
		m_VAOS.emplace_back(Sprite2DShader::CreateVAO());
	}

	Sprite2DRenderer::BatchRenderer::~BatchRenderer()
	{
		for (Sprite2DShader::Vao& v : m_VAOS){
			v.free();
		}
	}

	void Sprite2DRenderer::BatchRenderer::begin()
	{
		m_elements.clear();
	}

	void Sprite2DRenderer::BatchRenderer::end()
	{
		if (m_elements.empty())
			return;

		sortComponents();
		createRenderers();
	}

	void Sprite2DRenderer::BatchRenderer::drawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth)
	{
		m_elements.emplace_back(depth, sprite, modelMatrix);
	}

	void Sprite2DRenderer::BatchRenderer::render()
	{
		if (m_elements.empty())
			return;

		renderBatches();
	}

	void Sprite2DRenderer::BatchRenderer::sortComponents()
	{
		switch (m_sorting){
		case SpriteSortType::BY_DEPTH_TEXTURE:
			std::stable_sort(m_elements.begin(), m_elements.end(), compare_DepthTexture);
			break;

		default:
			break;
		}
	}

	void Sprite2DRenderer::BatchRenderer::createRenderers()
	{
		std::vector<glm::vec2> texAtexB;
		std::vector<glm::mat3> modelMatrices;

		texAtexB.reserve(m_elements.size() * 6); // 4 texcoord (x,y) + 2 (x,y) size/offset per glyph
		modelMatrices.reserve(m_elements.size());
		batches.clear();

		int offset = 0;

		const Sprite* lastSpr = nullptr;
		for (uint32 cg = 0; cg < m_elements.size(); cg++)
		{
			const Sprite* spr = m_elements[cg].sprite;
			const glm::vec4& uvrect = spr->getUV();

			if (spr != lastSpr 
				|| spr->getTexture() != lastSpr->getTexture() 
				|| spr->isTransparent() != lastSpr->isTransparent() )
			{
				batches.emplace_back(offset, 0, spr->getTexture(), spr->isTransparent());
			}
			offset += 1;
			batches.back().nItems += 1;
			lastSpr = spr;

			texAtexB.emplace_back(uvrect.x, uvrect.y); // BL  xw   zw
			texAtexB.emplace_back(uvrect.z, uvrect.y); // BR  
			texAtexB.emplace_back(uvrect.x, uvrect.w); // TL  
			texAtexB.emplace_back(uvrect.z, uvrect.w); // TR  xy   zy
			texAtexB.emplace_back(-spr->getOffsetX(), -spr->getOffsetY());
			texAtexB.emplace_back(spr->getWidth(), spr->getHeight());
			modelMatrices.emplace_back(*m_elements[cg].modelMatrix);
		}

		if (glewIsSupported("GL_VERSION_4_2") )
		{
			glBindBuffer(GL_ARRAY_BUFFER, m_VAOS[0].VBO[Sprite2DShader::VBO_VERTEXDATA]);
			glBufferData(GL_ARRAY_BUFFER, texAtexB.size() * sizeof(glm::vec2), texAtexB.data(), GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, m_VAOS[0].VBO[Sprite2DShader::VBO_MODELMAT]);
			glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat3), modelMatrices.data(), GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		} else {
			
			// Create more VAOs
			while (m_VAOS.size() < batches.size()){
				m_VAOS.emplace_back( Sprite2DShader::CreateVAO() );
			}

			// Bind data for each batch
			for (uint32 i = 0; i < batches.size(); ++i)
			{
				Batch& b = batches[i];

				glBindBuffer(GL_ARRAY_BUFFER, m_VAOS[i].VBO[Sprite2DShader::VBO_VERTEXDATA]);
				glBufferData(GL_ARRAY_BUFFER, b.nItems*6 * sizeof(glm::vec2), texAtexB.data() + (b.offset*6), GL_DYNAMIC_DRAW);

				glBindBuffer(GL_ARRAY_BUFFER, m_VAOS[i].VBO[Sprite2DShader::VBO_MODELMAT]);
				glBufferData(GL_ARRAY_BUFFER, b.nItems * sizeof(glm::mat3), modelMatrices.data() + b.offset, GL_DYNAMIC_DRAW);
			}

			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		check_gl_error();
	}

	void Sprite2DRenderer::BatchRenderer::renderBatches()
	{
		glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);

		if (glewIsSupported("GL_VERSION_4_2")){
			renderGL4();
		} else {
			renderGL3();
		}

		check_gl_error();
	}

	void Sprite2DRenderer::BatchRenderer::renderGL4()
	{
		glBindVertexArray(m_VAOS[0].VAO);

		for (const Batch& r : batches)
		{
			if (r.transparent){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			glBindTexture(GL_TEXTURE_2D, r.texture);

			glDrawArraysInstancedBaseInstance(GL_TRIANGLE_STRIP, 0, 4, r.nItems, r.offset);

			if (r.transparent){
				glDisable(GL_BLEND);
			}
		}

		glBindVertexArray(0);
	}

	void Sprite2DRenderer::BatchRenderer::renderGL3()
	{
		for (uint32 i = 0; i < batches.size(); ++i)
		{
			glBindVertexArray(m_VAOS[i].VAO);

			const Batch& r = batches[i];
			if (r.transparent){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			glBindTexture(GL_TEXTURE_2D, r.texture);

			glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, r.nItems);

			if (r.transparent){
				glDisable(GL_BLEND);
			}

		}

		glBindVertexArray(0);
	}

	bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(const RenderingElement& a, const RenderingElement& b){
		return a.depth < b.depth
			|| (a.depth == b.depth && (a.sprite->getTexture() < b.sprite->getTexture()));
	}

}