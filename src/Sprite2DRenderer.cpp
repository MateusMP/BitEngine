#include "Sprite2DRenderer.h"


#include <algorithm>

namespace BitEngine{


Sprite2DRenderer::Sprite2DRenderer(EntitySystem *sys, Transform2DProcessor* t2p, Camera2DProcessor* c2p)
	: es(sys), transform2Dprocessor(t2p), camera2Dprocessor(c2p)
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

std::vector<ComponentHandle>& Sprite2DRenderer::getComponents()
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
	// Find camera
	const Camera2DComponent* activeCamera = camera2Dprocessor->getActiveCamera();
	if (!activeCamera){
		LOGTO(Warning) << "Sprite2DRenderer: No active camera2D!" << endlog;
		return;
	}

	// Clear al batches
	for (BatchRenderer* r : m_batchRenderers){
		r->begin();
	}
	
	// Get all information needed -> Transform2DComponents
	const std::vector<ComponentHandle>& validComponents = components.getValidComponents();
	std::vector<ComponentHandle> search;
	std::vector<ComponentHandle> answer;
	std::vector<uint32> indices;
	es->findAllTuples<Sprite2DComponent, Transform2DComponent>(validComponents, answer, indices);

	// Culling
	for (uint32 i = 0; i < indices.size(); ++i){
		Transform2DComponent* t = (Transform2DComponent*)transform2Dprocessor->getComponent(answer[i]);
		Sprite2DComponent* spr = components.getComponent(validComponents[indices[i]]);

		if (insideScreen(t, spr)){
			m_batchRenderers[(int)spr->sortMode]->addComponent(t, spr);
		}
	}

	// Render
	shader->LoadViewMatrix(activeCamera->getMatrix());
	shader->Bind();
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

/// LOGIC

bool Sprite2DRenderer::insideScreen(const Transform2DComponent* t, const Sprite2DComponent* spr){

	return true;
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

void Sprite2DRenderer::BatchRenderer::addComponent(Transform2DComponent *t, Sprite2DComponent* c)
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
	std::vector<glm::mat3> modelMatrices;

	vertices.reserve(m_components.size() * 6); // 6 vertices per glyph
	batches.clear();

	int offset = 0;
	int cv = 0;
	const Transform2DComponent* t = m_components[0].transform;
	const Sprite2DComponent* c = m_components[0].spr;
	batches.emplace_back(offset, 6, c->sprite.textureID);
	vertices.emplace_back(0,			0 + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
	vertices.emplace_back(0,			0,				c->sprite.uvrect.x, c->sprite.uvrect.y);
	vertices.emplace_back(0 + c->width,	0,				c->sprite.uvrect.z, c->sprite.uvrect.y);
	vertices.emplace_back(0 + c->width,	0,				c->sprite.uvrect.z, c->sprite.uvrect.y);
	vertices.emplace_back(0 + c->width,	0 + c->height,	c->sprite.uvrect.z, c->sprite.uvrect.w);
	vertices.emplace_back(0,			0 + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
	modelMatrices.emplace_back(t->getMatrix());
	offset += 6;
	cv += 6;

	for (uint32 cg = 1; cg < m_components.size(); cg++)
	{
		const Transform2DComponent* t = m_components[cg].transform;
		const Sprite2DComponent* c = m_components[cg].spr;

		if (c->sprite.textureID != m_components[cg - 1].spr->sprite.textureID)
		{
			batches.emplace_back(offset, 6, c->sprite.textureID);
		}
		else {
			batches.back().nVertices += 6;
		}

		vertices.emplace_back(0,			0 + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
		vertices.emplace_back(0,			0,				c->sprite.uvrect.x,	c->sprite.uvrect.y);
		vertices.emplace_back(0 + c->width, 0,				c->sprite.uvrect.z,	c->sprite.uvrect.y);
		vertices.emplace_back(0 + c->width, 0,				c->sprite.uvrect.z,	c->sprite.uvrect.y);
		vertices.emplace_back(0 + c->width, 0 + c->height,	c->sprite.uvrect.z, c->sprite.uvrect.w);
		vertices.emplace_back(0,			0 + c->height,	c->sprite.uvrect.x, c->sprite.uvrect.w);
		modelMatrices.emplace_back(t->getMatrix());
		offset += 6;
	}

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[Sprite2DShader::VBO_VERTEX]);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Sprite2DShader::Vertex), nullptr, GL_DYNAMIC_DRAW); // TODO: verify best mode to use
	glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Sprite2DShader::Vertex), vertices.data());

	glBindBuffer(GL_ARRAY_BUFFER, m_vbo[Sprite2DShader::VBO_MODELMAT]);
	glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat3), nullptr, GL_DYNAMIC_DRAW); // TODO: verify best mode to use
	glBufferSubData(GL_ARRAY_BUFFER, 0, modelMatrices.size() * sizeof(glm::mat3), modelMatrices.data());

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void Sprite2DRenderer::BatchRenderer::renderBatches()
{
	glBindVertexArray(m_vao);

	glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);
	for (const Batch& r : batches)
	{
		glBindTexture(GL_TEXTURE_2D, r.texture);
		// glDrawArrays(GL_TRIANGLES, r.offset, r.nVertices);
		// glDrawElements(GL_TRIANGLES, 36, )
		// glDrawElementsInstanced(GL_TRIANGLES, r.nVertices, GL_UNSIGNED_INT, 0, m_components.size());
		glDrawArraysInstanced(GL_TRIANGLES, r.offset, 6, m_components.size());
	}

	check_gl_error();

	glBindVertexArray(0);
}

bool Sprite2DRenderer::BatchRenderer::compare_Texture(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->sprite.textureID < b.spr->sprite.textureID;
}

bool Sprite2DRenderer::BatchRenderer::compare_Depth(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->depth < b.spr->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_InvDepth(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->depth > b.spr->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_TextureDepth(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->sprite.textureID <= b.spr->sprite.textureID && a.spr->depth < b.spr->depth;
}

bool Sprite2DRenderer::BatchRenderer::compare_DepthTexture(const RenderingComponent& a, const RenderingComponent& b){
	return a.spr->depth <= b.spr->depth && a.spr->sprite.textureID < b.spr->sprite.textureID;
}


}