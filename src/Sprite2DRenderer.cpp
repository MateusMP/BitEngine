#include "Sprite2DRenderer.h"


#include <algorithm>

namespace BitEngine{


Sprite2DRenderer::Sprite2DRenderer(Camera2DProcessor* c2p)
	: camera2Dprocessor(c2p)
{
	
}

Sprite2DRenderer::~Sprite2DRenderer()
{
	delete shader;
}

Sprite2DShader* Sprite2DRenderer::getShader()
{
	return shader;
}

std::vector<Sprite2DComponent*>& Sprite2DRenderer::getComponents()
{
	return components.getValidComponents();
}

bool Sprite2DRenderer::Init()
{
	GLuint vbo[Sprite2DShader::NUM_VBOS];

	shader = new Sprite2DShader();
	shader->init();

	for (int i = 0; i < (int)Sprite2DComponent::SORT_TYPE::TOTAL; ++i){
		m_batchRenderers.push_back(new BatchRenderer((Sprite2DComponent::SORT_TYPE)i, shader->CreateVAO(vbo), vbo));
	}

	return true;
}

void Sprite2DRenderer::Process()
{
	for (BatchRenderer* r : m_batchRenderers)
		r->begin();

	const std::vector<Sprite2DComponent*>& validComponents = components.getValidComponents();
	const Camera2DComponent* activeCamera = camera2Dprocessor->getActiveCamera();

	if (!activeCamera){
		LOGTO(Warning) << "Sprite2DRenderer: No active camera2D!" << endlog;
		return;
	}

	shader->Bind();
	shader->LoadViewMatrix(activeCamera->getMatrix());

	for (Sprite2DComponent* s2c : validComponents)
	{
		m_batchRenderers[(int)s2c->sortMode]->addComponent(s2c);
	}

	for (BatchRenderer* b : m_batchRenderers)
	{
		b->render();
	}
}

ComponentHandle Sprite2DRenderer::CreateComponent()
{
	return components.newComponent();
}

void Sprite2DRenderer::DestroyComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Sprite2DRenderer::getComponent(ComponentHandle hdl)
{
	return components.getComponent(hdl);
}

/// ===============================================================================================
/// ===============================================================================================
///										GLYPH
/// ===============================================================================================

Sprite2DRenderer::BatchRenderer::BatchRenderer(Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS])
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
	m_components.clear();
}

void Sprite2DRenderer::BatchRenderer::addComponent(Sprite2DComponent* c)
{
	m_components.push_back(c);
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
		case Sprite2DComponent::SORT_TYPE::BY_TEXTURE_ONLY:
			std::stable_sort(m_components.begin(), m_components.end(), compare_Texture);
			break;
		case Sprite2DComponent::SORT_TYPE::BY_DEPTH_ONLY:
			std::stable_sort(m_components.begin(), m_components.end(), compare_Depth);
			break;
		case Sprite2DComponent::SORT_TYPE::BY_INVDEPTH_ONLY:
			std::stable_sort(m_components.begin(), m_components.end(), compare_InvDepth);
			break;
		case Sprite2DComponent::SORT_TYPE::BY_TEXTURE_DEPTH:
			std::stable_sort(m_components.begin(), m_components.end(), compare_TextureDepth);
			break;
		case Sprite2DComponent::SORT_TYPE::BY_DEPTH_TEXTURE:
			std::stable_sort(m_components.begin(), m_components.end(), compare_DepthTexture);
			break;

		default:
			break;
	}
}

void Sprite2DRenderer::BatchRenderer::createRenderers()
{
	std::vector<Sprite2DShader::Vertex> vertices;

	vertices.reserve(m_components.size() * 6); // 6 vertices per glyph
	batches.clear();

	int offset = 0;
	int cv = 0;
	Sprite2DComponent* c = m_components[0];
	batches.emplace_back(offset, 6, c->sprite.textureID);
	vertices.emplace_back(c->x,				c->y + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
	vertices.emplace_back(c->x,				c->y,				c->sprite.uvrect.x, c->sprite.uvrect.y);
	vertices.emplace_back(c->x+c->width,	c->y,				c->sprite.uvrect.z, c->sprite.uvrect.y);
	vertices.emplace_back(c->x + c->width,	c->y,				c->sprite.uvrect.z, c->sprite.uvrect.y);
	vertices.emplace_back(c->x + c->width,	c->y + c->height,	c->sprite.uvrect.z, c->sprite.uvrect.w);
	vertices.emplace_back(c->x,				c->y + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
	offset += 6;
	cv += 6;
	
	for (uint32 cg = 1; cg < m_components.size(); cg++)
	{
		Sprite2DComponent* c = m_components[cg];
		if (c->sprite.textureID != m_components[cg - 1]->sprite.textureID)
		{
			batches.emplace_back(offset, 6, c->sprite.textureID);
		}
		else {
			batches.back().nVertices += 6;
		}

		vertices.emplace_back(c->x, c->y + c->height, c->sprite.uvrect.x, c->sprite.uvrect.w);
		vertices.emplace_back(c->x, c->y, c->sprite.uvrect.x, c->sprite.uvrect.y);
		vertices.emplace_back(c->x + c->width, c->y, c->sprite.uvrect.z, c->sprite.uvrect.y);
		vertices.emplace_back(c->x + c->width, c->y, c->sprite.uvrect.z, c->sprite.uvrect.y);
		vertices.emplace_back(c->x + c->width, c->y + c->height, c->sprite.uvrect.z, c->sprite.uvrect.w);
		vertices.emplace_back(c->x, c->y + c->height, c->sprite.uvrect.x, c->sprite.uvrect.w);
		offset += 6;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Sprite2DShader::Vertex), nullptr, GL_DYNAMIC_DRAW); // TODO: verify best mode to use
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Sprite2DShader::Vertex), vertices.data());
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Sprite2DRenderer::BatchRenderer::renderBatches()
{
	glBindVertexArray(m_vao);

	glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);
	for (const Batch& r : batches)
	{
		glBindTexture(GL_TEXTURE_2D, r.texture);
		glDrawArrays(GL_TRIANGLES, r.offset, r.nVertices);
	}

	glBindVertexArray(0);
}

bool Sprite2DRenderer::BatchRenderer::compare_Texture(Sprite2DComponent* a, Sprite2DComponent* b){
	return a->sprite.textureID < b->sprite.textureID;
}

bool Sprite2DRenderer::BatchRenderer::compare_Depth(Sprite2DComponent* a, Sprite2DComponent* b){
	return a->depth < b->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_InvDepth(Sprite2DComponent* a, Sprite2DComponent* b){
	return a->depth > b->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_TextureDepth(Sprite2DComponent* a, Sprite2DComponent* b){
	return a->sprite.textureID <= b->sprite.textureID && a->depth < b->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(Sprite2DComponent* a, Sprite2DComponent* b){
	return a->depth <= b->depth && a->sprite.textureID < b->sprite.textureID;
}


}