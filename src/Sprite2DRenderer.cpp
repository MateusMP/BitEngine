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

	m_spriteManager = es->getResourceSystem()->getSpriteManager();

	for (int i = 0; i < (int)Sprite2DComponent::SORT_TYPE::TOTAL; ++i){
		m_batchRenderers.push_back(new BatchRenderer(m_spriteManager, (Sprite2DComponent::SORT_TYPE)i, shader->CreateVAO(vbo), vbo));
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

Sprite2DRenderer::BatchRenderer::BatchRenderer(SpriteManager* sprMng, Sprite2DComponent::SORT_TYPE s, GLuint vao, GLuint vbo[Sprite2DShader::NUM_VBOS])
	: m_spriteManager(sprMng), m_sorting(s), m_vao(vao)
{
	for (int i = 0; i < Sprite2DShader::NUM_VBOS; ++i){
		m_vbo[i] = vbo[i];
	}

	unsigned char INDEX_BUFFER[] = {
		0, 1, 2, // Bottom left triangle
		3		 // top right triangle
	};
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
		case Sprite2DComponent::SORT_TYPE::BY_DEPTH_TEXTURE:
			std::stable_sort(m_components.begin(), m_components.end(), compare_DepthTexture);
			break;

		default:
			break;
	}
}

static int totalitems = 0;
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
	batches.emplace_back(0, 1, spr->textureID, spr->transparent);
	texAtexB.emplace_back(spr->uvrect.x, spr->uvrect.y); // BL  xw   zw
	texAtexB.emplace_back(spr->uvrect.x, spr->uvrect.w); // TL  
	texAtexB.emplace_back(spr->uvrect.z, spr->uvrect.y); // BR  
	texAtexB.emplace_back(spr->uvrect.z, spr->uvrect.w); // TR  xy   zy
	texAtexB.emplace_back(-spr->offsetX, -spr->offsetY);
	texAtexB.emplace_back(spr->width, spr->height);
	modelMatrices.emplace_back(t->getMatrix());
	totalitems = 1;
	const Sprite* lastSpr = spr;
	for (uint32 cg = 1; cg < m_components.size(); cg++)
	{
		const Transform2DComponent* t = m_components[cg].transform;
		const Sprite2DComponent* c = m_components[cg].spr;
		const Sprite* spr = m_spriteManager->getSprite(c->sprite);

		offset += 1;

		if (spr->textureID != lastSpr->textureID || spr->transparent != lastSpr->transparent)
		{
			batches.emplace_back(offset, 0, spr->textureID, spr->transparent);
		}
		batches.back().nItems += 1;
		lastSpr = spr;

		texAtexB.emplace_back(spr->uvrect.x, spr->uvrect.y); // BL  xw   zw
		texAtexB.emplace_back(spr->uvrect.x, spr->uvrect.w); // TL  
		texAtexB.emplace_back(spr->uvrect.z, spr->uvrect.y); // BR  
		texAtexB.emplace_back(spr->uvrect.z, spr->uvrect.w); // TR  xy   zy
		texAtexB.emplace_back(-spr->offsetX, -spr->offsetY);
		texAtexB.emplace_back(spr->width, spr->height);
		modelMatrices.emplace_back(t->getMatrix());

		++totalitems;
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