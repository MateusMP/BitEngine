#include "Sprite2DProcessor.h"

#include <algorithm>

namespace BitEngine{


Sprite2DProcessor::Sprite2DProcessor()
{
}

Sprite2DProcessor::~Sprite2DProcessor()
{
}

const std::vector<ComponentHandle>& Sprite2DProcessor::getComponents() const
{
	return components.getValidComponents();
}

bool Sprite2DProcessor::Init()
{
	return true;
}

void Sprite2DProcessor::FrameEnd()
{
	// Do nothing.
	return;
}

ComponentHandle Sprite2DProcessor::CreateComponent(EntityHandle entity)
{
	return components.newComponent();
}

void Sprite2DProcessor::DestroyComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Sprite2DProcessor::getComponent(ComponentHandle hdl)
{
	return components.getComponent(hdl);
}

///

/// ===============================================================================================
/// ===============================================================================================
///										BATCH
/// ===============================================================================================

Sprite2DRenderer::Sprite2DRenderer(const EntitySystem* _es)
	: m_es(_es)
{
}

bool Sprite2DRenderer::Init()
{
	GLuint vbo[Sprite2DShader::NUM_VBOS];

	shader = new Sprite2DShader();
	if (shader->Init() <= 0)
		return false;

	m_spriteManager = m_es->getResourceSystem()->getSpriteManager();

	for (int i = 0; i < (int)Sprite2DComponent::SORT_TYPE::TOTAL; ++i){
		m_batchRenderers.push_back(new BatchRenderer(m_spriteManager, (Sprite2DComponent::SORT_TYPE)i, shader->CreateVAO(vbo), vbo));
	}

	return true;
}

Sprite2DShader* Sprite2DRenderer::getShader()
{
	return shader;
}

void Sprite2DRenderer::clearRenderer()
{
	// Clear all batches
	for (BatchRenderer* r : m_batchRenderers){
		r->begin();
	}
}

void Sprite2DRenderer::addToRender(const Sprite2DComponent* sprite, const Transform2DComponent* transform)
{
	m_batchRenderers[(int)sprite->sortMode]->addComponent(transform, sprite);
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

Sprite2DRenderer::BatchRenderer::BatchRenderer(SpriteManager* sprMng, Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS])
	: m_spriteManager(sprMng), m_sorting(s), m_vao(vao)
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
	m_components.clear();
}

void Sprite2DRenderer::BatchRenderer::addComponent(const Transform2DComponent *t, const Sprite2DComponent* c)
{
	m_components.emplace_back(t, c);
}

void Sprite2DRenderer::BatchRenderer::render()
{
	if (m_components.empty())
		return;

	sortComponents();
	createRenderers();
	renderBatches();
}

void Sprite2DRenderer::BatchRenderer::sortComponents()
{
	switch (m_sorting){
		case Sprite2DComponent::SORT_TYPE::BY_DEPTH_TEXTURE:
			std::stable_sort(m_components.begin(), m_components.end(), compare_DepthTexture);
			break;

		default:
			break;
	}
}

void Sprite2DRenderer::BatchRenderer::createRenderers()
{
	std::vector<glm::vec2> texAtexB;
	std::vector<glm::mat3> modelMatrices;

	texAtexB.reserve(m_components.size() * 4); // 4 texcoord (x,y) per glyph
	batches.clear();

	int offset = 0;
	const Transform2DComponent* t = m_components[0].transform;
	const Sprite2DComponent* c = m_components[0].spr;
	const Sprite* spr = m_spriteManager->getSprite(c->sprite);
	const glm::vec4& uvrect = spr->getUV();
	batches.emplace_back(0, 1, spr->getTexture(), spr->isTransparent());
	texAtexB.emplace_back(uvrect.x, uvrect.y); // BL  xw   zw
	texAtexB.emplace_back(uvrect.z, uvrect.y); // BR  
	texAtexB.emplace_back(uvrect.x, uvrect.w); // TL  
	texAtexB.emplace_back(uvrect.z, uvrect.w); // TR  xy   zy
	texAtexB.emplace_back(-spr->getOffsetX(), -spr->getOffsetY());
	texAtexB.emplace_back(spr->getWidth(), spr->getHeight());
	modelMatrices.emplace_back(t->getMatrix());

	const Sprite* lastSpr = spr;
	for (uint32 cg = 1; cg < m_components.size(); cg++)
	{
		const Transform2DComponent* t = m_components[cg].transform;
		const Sprite2DComponent* c = m_components[cg].spr;
		const Sprite* spr = m_spriteManager->getSprite(c->sprite);
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
		modelMatrices.emplace_back(t->getMatrix());
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

bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->depth < b.spr->depth
		|| (a.spr->depth == b.spr->depth && (a.spr->sprite < b.spr->sprite));
}


}