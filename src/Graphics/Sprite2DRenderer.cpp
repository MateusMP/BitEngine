#include "Sprite2DRenderer.h"

namespace BitEngine{

	Sprite2DRenderer::Sprite2DRenderer()
	{
	}

	Sprite2DRenderer::~Sprite2DRenderer()
	{
		for (BatchRenderer* b : m_batchRenderers){
			delete b;
		}
	}

	void Sprite2DRenderer::Init()
	{
		for (int i = 0; i < (int)SpriteSortType::TOTAL; ++i){
			m_batchRenderers.push_back(new BatchRenderer((SpriteSortType)i));
		}
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
		RENDERER_VERSION = Sprite2DShader::GetRendererVersion();

		if (RENDERER_VERSION == Sprite2DShader::USE_GL4)
		{
			// Make sure we have the vao needed
			m_interVAOs.emplace_back();
			m_interVAOs.back().Create();
		}
	}

	Sprite2DRenderer::BatchRenderer::~BatchRenderer()
	{
		for (auto& v : m_interVAOs){
			v.Destroy();
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
		if (RENDERER_VERSION == Sprite2DShader::RendererVersion::USE_GL4
			|| RENDERER_VERSION == Sprite2DShader::RendererVersion::USE_GL3)
		{
			createRenderersGL4GL3();
		}
		else if (RENDERER_VERSION == Sprite2DShader::RendererVersion::USE_GL2){
			createRenderersGL2();
		}

		check_gl_error();
	}

	void Sprite2DRenderer::BatchRenderer::createRenderersGL2()
	{
		std::vector<Sprite2Dbasic_VD::Data> vertices_;

		const uint32 NUM_VERTS = 6;

		vertices_.resize(m_elements.size()*NUM_VERTS); // Generates NUM_VERTS vertices for each element

		batches.clear();

		const glm::vec2 vertex_pos[4] = {
			glm::vec2(0.0f, 0.0f),
			glm::vec2(1.0f, 0.0f),
			glm::vec2(0.0f, 1.0f),
			glm::vec2(1.0f, 1.0f)
		};

		int offset = 0;
		const Sprite* lastSpr = nullptr;
		for (uint32 cg = 0; cg < m_elements.size(); cg++)
		{
			const uint32 vertexID = cg*NUM_VERTS;
			const Sprite* spr = m_elements[cg].sprite;
			const glm::vec4& uvrect = spr->getUV();

			if (spr != lastSpr
			|| spr->getTexture() != lastSpr->getTexture()
			|| spr->isTransparent() != lastSpr->isTransparent())
			{
				batches.emplace_back(offset, 0, spr->getTexture(), spr->isTransparent());
			}
			offset += NUM_VERTS;
			batches.back().nItems += NUM_VERTS;
			lastSpr = spr;

			const glm::mat3& modelMatrix = *m_elements[cg].modelMatrix;
			const glm::vec2 sizes(spr->getWidth(), spr->getHeight());
			const glm::vec2 offsets(-spr->getOffsetX(), -spr->getOffsetY());
			const glm::vec2 off_siz = offsets*sizes;

			// pos
			vertices_[vertexID + 0].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[0] * sizes + off_siz, 1));
			vertices_[vertexID + 1].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
			vertices_[vertexID + 2].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
			vertices_[vertexID + 3].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
			vertices_[vertexID + 4].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
			vertices_[vertexID + 5].vertexPos = glm::vec2(modelMatrix * glm::vec3(vertex_pos[3] * sizes + off_siz, 1));
			
			// uvs
			vertices_[vertexID+0].vertexUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw
			vertices_[vertexID+1].vertexUV = glm::vec2(uvrect.z, uvrect.y); // BR  
			vertices_[vertexID+2].vertexUV = glm::vec2(uvrect.x, uvrect.w); // TL  
			vertices_[vertexID+3].vertexUV = glm::vec2(uvrect.x, uvrect.w); // TL  
			vertices_[vertexID+4].vertexUV = glm::vec2(uvrect.z, uvrect.y); // BR  
			vertices_[vertexID+5].vertexUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy
		}

		// Upload data to gpu

		// Create more VAOs if needed
		while (m_basicVAOs.size() < batches.size()){
			m_basicVAOs.emplace_back();
			m_basicVAOs.back().Create();
		}

		// Bind data for each batch
		for (uint32 i = 0; i < batches.size(); ++i)
		{
			Batch& b = batches[i];

			m_basicVAOs[i].IVBO<Sprite2Dbasic_VD>::vbo.BindBuffer();
			m_basicVAOs[i].IVBO<Sprite2Dbasic_VD>::vbo.LoadBuffer(&vertices_[b.offset], b.nItems);
		}
		IVertexArrayBuffer::UnbindBuffer();
		
	}

	void Sprite2DRenderer::BatchRenderer::createRenderersGL4GL3()
	{
		std::vector<Sprite2Dinstanced_VDVertices::Data> vertices_;
		std::vector<Sprite2Dinstanced_VDModelMatrix::Data> modelMatrices_;

		vertices_.resize(m_elements.size());
		modelMatrices_.resize(m_elements.size());

		batches.clear();

		int offset = 0;
		const Sprite* lastSpr = nullptr;
		for (uint32 cg = 0; cg < m_elements.size(); cg++)
		{
			const Sprite* spr = m_elements[cg].sprite;
			const glm::vec4& uvrect = spr->getUV();

			if (spr != lastSpr
				|| spr->getTexture() != lastSpr->getTexture()
				|| spr->isTransparent() != lastSpr->isTransparent())
			{
				batches.emplace_back(offset, 0, spr->getTexture(), spr->isTransparent());
			}
			offset += 1;
			batches.back().nItems += 1;
			lastSpr = spr;

			vertices_[cg].tex_coord[0] = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw
			vertices_[cg].tex_coord[1] = glm::vec2(uvrect.z, uvrect.y); // BR  
			vertices_[cg].tex_coord[2] = glm::vec2(uvrect.x, uvrect.w); // TL  
			vertices_[cg].tex_coord[3] = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy
			vertices_[cg].size_offset = glm::vec4(-spr->getOffsetX(), -spr->getOffsetY(), spr->getWidth(), spr->getHeight());
			modelMatrices_[cg].modelMatrix = (*m_elements[cg].modelMatrix);
		}

		// Upload data to gpu
		if (RENDERER_VERSION == Sprite2DShader::USE_GL4)
		{
			m_interVAOs[0].IVBO<Sprite2Dinstanced_VDVertices>::vbo.BindBuffer();
			m_interVAOs[0].IVBO<Sprite2Dinstanced_VDVertices>::vbo.LoadBuffer(vertices_.data(), vertices_.size());

			m_interVAOs[0].IVBO<Sprite2Dinstanced_VDModelMatrix>::vbo.BindBuffer();
			m_interVAOs[0].IVBO<Sprite2Dinstanced_VDModelMatrix>::vbo.LoadBuffer(modelMatrices_.data(), modelMatrices_.size());

			IVertexArrayBuffer::UnbindBuffer();
		}
		else if (RENDERER_VERSION == Sprite2DShader::USE_GL3)
		{
			// Create more VAOs if needed
			while (m_interVAOs.size() < batches.size()){
				m_interVAOs.emplace_back();
				m_interVAOs.back().Create();
			}

			// Bind data for each batch
			for (uint32 i = 0; i < batches.size(); ++i)
			{
				Batch& b = batches[i];

				m_interVAOs[i].IVBO<Sprite2Dinstanced_VDVertices>::vbo.BindBuffer();
				m_interVAOs[i].IVBO<Sprite2Dinstanced_VDVertices>::vbo.LoadBuffer(&vertices_[b.offset], b.nItems);

				m_interVAOs[i].IVBO<Sprite2Dinstanced_VDModelMatrix>::vbo.BindBuffer();
				m_interVAOs[i].IVBO<Sprite2Dinstanced_VDModelMatrix>::vbo.LoadBuffer(&modelMatrices_[b.offset], b.nItems);
			}
			IVertexArrayBuffer::UnbindBuffer();
		}
	}

	void Sprite2DRenderer::BatchRenderer::renderBatches()
	{
		glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);

		if (RENDERER_VERSION == Sprite2DShader::USE_GL4){
			renderGL4();
		}
		else if (RENDERER_VERSION == Sprite2DShader::USE_GL3){
			renderGL3();
		}
		else if (RENDERER_VERSION == Sprite2DShader::USE_GL2){
			renderGL2();
		}

		check_gl_error();
	}

	void Sprite2DRenderer::BatchRenderer::renderGL4()
	{
		m_interVAOs[0].Bind();

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

		IVertexArrayObject::Unbind();
	}

	void Sprite2DRenderer::BatchRenderer::renderGL3()
	{
		for (uint32 i = 0; i < batches.size(); ++i)
		{
			m_interVAOs[i].Bind();
			
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

		IVertexArrayObject::Unbind();
	}

	void Sprite2DRenderer::BatchRenderer::renderGL2()
	{
		for (uint32 i = 0; i < batches.size(); ++i)
		{
			m_basicVAOs[i].Bind();

			const Batch& r = batches[i];
			if (r.transparent){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			glBindTexture(GL_TEXTURE_2D, r.texture);

			glDrawArrays(GL_TRIANGLES, 0, r.nItems);

			if (r.transparent){
				glDisable(GL_BLEND);
			}

		}

		IVertexArrayObject::Unbind();
	}

	bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(const RenderingElement& a, const RenderingElement& b){
		return a.depth < b.depth
			|| (a.depth == b.depth && (a.sprite->getTexture() < b.sprite->getTexture()));
	}

}