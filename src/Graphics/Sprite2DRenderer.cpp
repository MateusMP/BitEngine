#include "Sprite2DRenderer.h"

namespace BitEngine{

	Sprite2DRenderer::Sprite2DRenderer()
	{
	}

	bool Sprite2DRenderer::Init()
	{
		GLuint vbo[Sprite2DShader::NUM_VBOS];

		shader = new Sprite2DShader();
		if (shader->Init() <= 0)
			return false;

		for (int i = 0; i < (int)SpriteSortType::TOTAL; ++i){
			m_batchRenderers.push_back(new BatchRenderer((SpriteSortType)i, shader->CreateVAO(vbo), vbo));
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
		// shader->LoadViewMatrix(activeCamera->getMatrix());
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

	Sprite2DRenderer::BatchRenderer::BatchRenderer(SpriteSortType s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS])
		: m_sorting(s), m_vao(vao)
	{
		for (int i = 0; i < Sprite2DShader::NUM_VBOS; ++i){
			m_vbo[i] = vbo[i];
		}
	}

	Sprite2DRenderer::BatchRenderer::~BatchRenderer()
	{
		if (m_vao != 0){
			glDeleteVertexArrays(1, &m_vao);
		}

		if (m_vbo[0] != 0){
			glDeleteBuffers(Sprite2DShader::NUM_VBOS, m_vbo);
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
		batches.clear();

		int offset = 0;
		const Sprite* spr = m_elements[0].sprite;
		const glm::vec4& uvrect = spr->getUV();
		batches.emplace_back(0, 1, spr->getTexture(), spr->isTransparent());
		texAtexB.emplace_back(uvrect.x, uvrect.y); // BL  xw   zw
		texAtexB.emplace_back(uvrect.z, uvrect.y); // BR  
		texAtexB.emplace_back(uvrect.x, uvrect.w); // TL  
		texAtexB.emplace_back(uvrect.z, uvrect.w); // TR  xy   zy
		texAtexB.emplace_back(-spr->getOffsetX(), -spr->getOffsetY());
		texAtexB.emplace_back(spr->getWidth(), spr->getHeight());
		modelMatrices.emplace_back(*m_elements[0].modelMatrix);

		const Sprite* lastSpr = spr;
		for (uint32 cg = 1; cg < m_elements.size(); cg++)
		{
			const Sprite* spr = m_elements[cg].sprite;
			const glm::vec4& uvrect = spr->getUV();

			offset += 1;

			if (spr->getTexture() != lastSpr->getTexture() || spr->isTransparent() != lastSpr->isTransparent())
			{
				batches.emplace_back(offset, 0, spr->getTexture(), spr->isTransparent());
			}
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

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[Sprite2DShader::VBO_VERTEXDATA]);
		glBufferData(GL_ARRAY_BUFFER, texAtexB.size() * sizeof(glm::vec2), texAtexB.data(), GL_DYNAMIC_DRAW); // TODO: verify best mode to use

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[Sprite2DShader::VBO_MODELMAT]);
		glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat3), modelMatrices.data(), GL_DYNAMIC_DRAW); // TODO: verify best mode to use

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		check_gl_error();
	}


	void Sprite2DRenderer::BatchRenderer::renderBatches()
	{
		glBindVertexArray(m_vao);

		glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);
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

		check_gl_error();
	}

	bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(const RenderingElement& a, const RenderingElement& b){
		return a.depth < b.depth
			|| (a.depth == b.depth && (a.sprite->getTexture() < b.sprite->getTexture()));
	}

}