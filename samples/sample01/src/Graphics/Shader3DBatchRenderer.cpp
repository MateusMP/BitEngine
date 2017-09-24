#include "Graphics/Shader3DSimple.h"

// ******************* BatchRendererGL4 *******************

Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::BatchRendererGL4(){
	glGenBuffers(1, &modelMatrixVBO);
}

Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::~BatchRendererGL4(){
	glDeleteBuffers(1, &modelMatrixVBO);
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::addMesh(Mesh* mesh, const Material* mat, const glm::mat4* modelMat) {
	data[mesh].emplace_back(mat, modelMat);
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::Begin() {
	data.clear();
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::End() {
	sort();
	createBatches();
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::Render() {
	GLuint lastVAO = 0;
	GLuint lastDiffuse = 0;
	GLuint lastNormal = 0;
	for (Batch& b : batches)
	{
		// BIND VAO
		if (lastVAO != b.mesh->getVAO()){
			glBindVertexArray(b.mesh->getVAO());

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.mesh->getVBO()[VBO_INDEX]);
		}

		for (const Batch& r : batches)
		{
			if (r.transparent){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			// Bind DIFFUSE
			const GLuint diffuse = r.material->diffuse->getTextureID();
			if (lastDiffuse != diffuse){
				glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_SLOT);
				glBindTexture(GL_TEXTURE_2D, diffuse);
				lastDiffuse = diffuse;
			}

			// Bind NORMAL
			const GLuint normal = r.material->normal->getTextureID();
			if (lastNormal != normal){
				glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_SLOT);
				glBindTexture(GL_TEXTURE_2D, normal);
				lastNormal = normal;
			}

			// DRAW
			glDrawElementsInstancedBaseInstance(GL_TRIANGLES, r.mesh->getNumIndices(), GL_UNSIGNED_INT, 0, r.nItems, r.offset);

			if (r.transparent){
				glDisable(GL_BLEND);
			}
		}

		//check_gl_error();
	}

	// FREE VAO
	glBindVertexArray(0);
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::sort()
{
	for (auto& it : data)
	{
		std::vector<RenderData>& v = it.second;
		std::sort(v.begin(), v.end(), sortRenderData);
	}
}

void Shader3DSimple::Shader3DSimpleGL4::BatchRendererGL4::createBatches()
{
	batches.clear();

	const Mesh* lastMesh = nullptr;
	const Material* lastMaterial = nullptr;
	for (auto& it : data)
	{
		std::vector<glm::mat4> modelMatrices;
		const std::vector<RenderData>& v = it.second;
		int offset = 0;

		for (u32 i = 0; i < v.size(); ++i)
		{
			const RenderData& d = v[i];
			if (it.first != lastMesh || lastMaterial != d.material)
			{
				batches.emplace_back(offset, 0, it.first, d.material, false);
				lastMesh = it.first;
				lastMaterial = d.material;
			}

			modelMatrices.emplace_back(*(v[i].modelMatrix));
			// modelMatrices.emplace_back(glm::mat4());
			batches.back().nItems++;
			offset++;
		}

		glBindBuffer(GL_ARRAY_BUFFER, it.first->getVBO()[VBO_MODELMAT]);
		glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_DYNAMIC_DRAW); // TODO: verify best mode to use
	}
}


// ******************* BatchRendererGL2 *******************

Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::BatchRendererGL2(Shader3DSimple::Shader3DSimpleGL2 *shader)
	: m_shader(shader)
{
}

Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::~BatchRendererGL2(){
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::addMesh(Mesh* mesh, const Material* mat, const glm::mat4* modelMat) {
	data[mesh].emplace_back(mat, modelMat);
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::Begin() {
	data.clear();
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::End() {
	sort();
	createBatches();
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::Render() {
	GLuint lastVAO = 0;
	GLuint lastDiffuse = 0;
	GLuint lastNormal = 0;

	int modelMatrixIndex = 0;
	for (Batch& b : batches)
	{
		// BIND VAO
		if (lastVAO != b.mesh->getVAO()){
			glBindVertexArray(b.mesh->getVAO());

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, b.mesh->getVBO()[VBO_INDEX]);
		}

		for (const Batch& r : batches)
		{
			if (r.transparent){
				glEnable(GL_BLEND);
				glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}

			// Bind DIFFUSE
			const GLuint diffuse = r.material->diffuse->getTextureID();
			if (lastDiffuse != diffuse){
				glActiveTexture(GL_TEXTURE0 + DIFFUSE_TEXTURE_SLOT);
				glBindTexture(GL_TEXTURE_2D, diffuse);
				lastDiffuse = diffuse;
			}

			// Bind NORMAL
			const GLuint normal = r.material->normal->getTextureID();
			if (lastNormal != normal){
				glActiveTexture(GL_TEXTURE0 + NORMAL_TEXTURE_SLOT);
				glBindTexture(GL_TEXTURE_2D, normal);
				lastNormal = normal;
			}

			// DRAW
			for (int i = 0; i < b.nItems; ++i){

				// Load Model Matrix
				m_shader->LoadIntanceModelMatrix(modelMatrices[modelMatrixIndex]);
				modelMatrixIndex++;

				glDrawElements(GL_TRIANGLES, r.mesh->getNumIndices(), GL_UNSIGNED_INT, 0 );
			}

			if (r.transparent){
				glDisable(GL_BLEND);
			}
		}

		//check_gl_error();
	}

	// FREE VAO
	glBindVertexArray(0);
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::sort()
{
	for (auto& it : data)
	{
		std::vector<RenderData>& v = it.second;
		std::sort(v.begin(), v.end(), sortRenderData);
	}
}

void Shader3DSimple::Shader3DSimpleGL2::BatchRendererGL2::createBatches()
{
	batches.clear();

	const Mesh* lastMesh = nullptr;
	const Material* lastMaterial = nullptr;
	for (auto& it : data)
	{
		const std::vector<RenderData>& v = it.second;
		int offset = 0;

		for (u32 i = 0; i < v.size(); ++i)
		{
			const RenderData& d = v[i];
			if (it.first != lastMesh || lastMaterial != d.material)
			{
				batches.emplace_back(offset, 0, it.first, d.material, false);
				lastMesh = it.first;
				lastMaterial = d.material;
			}

			modelMatrices.emplace_back(*(v[i].modelMatrix));
			batches.back().nItems++;
			offset++;
		}
	}
}
