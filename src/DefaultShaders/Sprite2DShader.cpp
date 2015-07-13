#include "DefaultShaders/Sprite2DShader.h"

#include <algorithm>

#define GLSL(version, shader)  "#version " #version "\n" #shader

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* sprite2DshaderVertex = GLSL(150,

	in vec2 a_vertexPosition;												
	in vec2 a_textureCoord;													
																			
	out vec2 fragTextureCoord;

	uniform mat4 u_viewMatrix;
																			
	void main()																
	{																		
		gl_Position.xy = (u_viewMatrix * vec4(a_vertexPosition, 0, 1.0f)).xy;
		gl_Position.z = 0.0;												
		gl_Position.w = 1.0;												
																			
		fragTextureCoord = a_textureCoord;									
	}												
);

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* sprite2DshaderFragment = GLSL(150,							
	in vec2 fragTextureCoord;													
																				
	out vec4 finalColor;														
																				
	uniform sampler2D u_texDiffuse;												
																				
	void main()																	
	{																			
		finalColor = texture(u_texDiffuse, fragTextureCoord);					
	}
);
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace BitEngine{

	Sprite2DShader::Sprite2DShader()
	{
	}


	Sprite2DShader::~Sprite2DShader()
	{
	}

	int Sprite2DShader::init(){
		// return CompileShadersFiles("Shaders/vertex.glsl", "Shaders/fragment.glsl");
		return CompileShadersSources(sprite2DshaderVertex, sprite2DshaderFragment);
	}

	void Sprite2DShader::LoadViewMatrix(glm::mat4& matrix)
	{
		u_viewMatrix = matrix;
	}

	Sprite2DShader::Sprite2DBatch* Sprite2DShader::Create2DBatchRenderer(IBatchRenderer::BATCH_MODE mode)
	{
		GLuint newVao;
		GLuint newVbos[NUM_VBOS];

		newVao = CreateVAO(newVbos);
		Sprite2DBatch* batch = new Sprite2DBatch(newVao, newVbos, mode, this);

		return batch;
	}


	void Sprite2DShader::BindAttributes() {
		BindAttribute(ATTR_VERTEX_POS, "a_vertexPosition");
		BindAttribute(ATTR_VERTEX_TEX, "a_textureCoord");

		check_gl_error();
	}

	void Sprite2DShader::RegisterUniforms() {
		LOAD_UNIFORM(u_texDiffuseHdl, "u_texDiffuse");
		LOAD_UNIFORM(u_viewMatrixHdl, "u_viewMatrix");
	}

	void Sprite2DShader::OnBind() {
		connectTexture(u_texDiffuseHdl, TEXTURE_DIFFUSE);

		loadMatrix(u_viewMatrixHdl, &(u_viewMatrix[0][0]) );
	}

	GLuint Sprite2DShader::CreateVAO(GLuint* outVBO) {
		GLuint vao;

		// VAO
		glGenVertexArrays(1, &vao);
		if (vao == 0){
			LOGTO(Error) << "Sprite2DBatch: Could not create VAO." << endlog;
		}
		glBindVertexArray(vao);

		// VBO
		glGenBuffers(NUM_VBOS, outVBO);
		if (outVBO[0] == 0){
			LOGTO(Error) << "Sprite2DBatch: Could not create VBO." << endlog;
		}
		glBindBuffer(GL_ARRAY_BUFFER, outVBO[0]);

		// Attributes
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindVertexArray(0);

		return vao;
	}


	/// ===============================================================================================
	/// ===============================================================================================
	///										Sprite2DBatch
	/// ===============================================================================================

	Sprite2DShader::Sprite2DBatch::Glyph::Glyph(const glm::vec2& _pos, const Sprite& _sprite, int width, int height, float _depth)
		: textureID(_sprite.textureID), depth(_depth)
	{
		/**
		xw--zw (w,h)
		|    |
		xy--zy
		(0,0)
		*/

		topleft.position.x = _pos.x;
		topleft.position.y = _pos.y + height;
		topleft.uv.u = _sprite.uvrect.x;
		topleft.uv.v = _sprite.uvrect.w;

		bottomleft.position.x = _pos.x;
		bottomleft.position.y = _pos.y;
		bottomleft.uv.u = _sprite.uvrect.x;
		bottomleft.uv.v = _sprite.uvrect.y;

		topright.position.x = _pos.x + width;
		topright.position.y = _pos.y + height;
		topright.uv.u = _sprite.uvrect.z;
		topright.uv.v = _sprite.uvrect.w;

		bottomright.position.x = _pos.x + width;
		bottomright.position.y = _pos.y;
		bottomright.uv.u = _sprite.uvrect.z;
		bottomright.uv.v = _sprite.uvrect.y;
	}

	Sprite2DShader::Sprite2DBatch::Glyph::Glyph(const glm::vec2& _pos, const Sprite& _sprite, float _depth)
		: textureID(_sprite.textureID), depth(_depth)
	{
		/**
		xw--zw (1,1)
		|    |
		xy--zy
		(0,0)
		*/

		topleft.position.x = _pos.x;
		topleft.position.y = _pos.y + 1;
		topleft.uv.u = _sprite.uvrect.x;
		topleft.uv.v = _sprite.uvrect.w;

		bottomleft.position.x = _pos.x;
		bottomleft.position.y = _pos.y;
		bottomleft.uv.u = _sprite.uvrect.x;
		bottomleft.uv.v = _sprite.uvrect.y;

		topright.position.x = _pos.x + 1;
		topright.position.y = _pos.y + 1;
		topright.uv.u = _sprite.uvrect.z;
		topright.uv.v = _sprite.uvrect.w;

		bottomright.position.x = _pos.x + 1;
		bottomright.position.y = _pos.y;
		bottomright.uv.u = _sprite.uvrect.z;
		bottomright.uv.v = _sprite.uvrect.y;
	}

	Sprite2DShader::Sprite2DBatch::Sprite2DBatch(GLuint _vao, GLuint _vbos[NUM_VBOS], BATCH_MODE _mode, ShaderProgram* _shader)
		: m_vao(_vao), m_sort(SORT_TYPE::BY_TEXTURE_ONLY), m_mode(_mode), shader(_shader)
	{
		// Copy vbos IDs
		for (int i = 0; i < NUM_VBOS; ++i)
			m_vbo[i] = _vbos[i];

		if (m_mode == BATCH_MODE::DYNAMIC){
			glMODE = GL_DYNAMIC_DRAW;
		}else if (m_mode == BATCH_MODE::STATIC){
			glMODE = GL_STATIC_DRAW;
		}else {
			glMODE = GL_STREAM_DRAW;
		}

	}

	Sprite2DShader::Sprite2DBatch::~Sprite2DBatch()
	{
		if (m_vao != 0){
			glDeleteVertexArrays(1, &m_vao);
		}

		if (m_vbo[0] != 0){
			glDeleteBuffers(NUM_VBOS, m_vbo);
		}
	}

	void Sprite2DShader::Sprite2DBatch::setSortingType(SORT_TYPE type)
	{
		m_sort = type;
	}

	void Sprite2DShader::Sprite2DBatch::DrawSprite(const glm::vec2& pos, const Sprite& sprite, int width, int height, float depth){

		m_glyphsData.emplace_back(pos, sprite, width, height, depth);
		m_glyphs.push_back(&m_glyphsData[m_glyphsData.size() - 1]);

	}

	void Sprite2DShader::Sprite2DBatch::begin(){
		if (m_mode == BATCH_MODE::STATIC_DEFINED){
			LOGTO(Warning) << "Shader: Trying to modify STATIC batch!" << endlog;
			return;
		}

		m_glyphs.clear();
		m_glyphsData.clear();
	}

	void Sprite2DShader::Sprite2DBatch::end(){
		if (m_mode == BATCH_MODE::STATIC_DEFINED){
			LOGTO(Warning) << "Shader: Trying to modify STATIC batch!" << endlog;
			return;
		}

		sortGlyphs();
		createRenderers();

		// Batch baked!
		if (m_mode == BATCH_MODE::STATIC)
			m_mode = BATCH_MODE::STATIC_DEFINED;
	}

	void Sprite2DShader::Sprite2DBatch::Render()
	{
		glBindVertexArray(m_vao);

		glActiveTexture(GL_TEXTURE0 + TEXTURE_DIFFUSE);
		for (const Renderer& r : batchRenderers)
		{
			glBindTexture(GL_TEXTURE_2D, r.texture);
			glDrawArrays(GL_TRIANGLES, r.offset, r.nVertices);
		}

		glBindVertexArray(0);
	}

	ShaderProgram* Sprite2DShader::Sprite2DBatch::getShader()
	{
		return shader;
	}

	void Sprite2DShader::Sprite2DBatch::createRenderers()
	{
		std::vector<Vertex> vertices;

		if (m_glyphs.empty())
			return;

		vertices.resize(m_glyphs.size() * 6); // 6 vertices per glyph
		batchRenderers.clear();

		int offset = 0;
		int cv = 0;
		batchRenderers.emplace_back(offset, 6, m_glyphs[0]->textureID);
		vertices[cv++] = m_glyphs[0]->topleft;
		vertices[cv++] = m_glyphs[0]->bottomleft;
		vertices[cv++] = m_glyphs[0]->bottomright;
		vertices[cv++] = m_glyphs[0]->bottomright;
		vertices[cv++] = m_glyphs[0]->topright;
		vertices[cv++] = m_glyphs[0]->topleft;
		offset += 6;

		for (uint32 cg = 1; cg < m_glyphs.size(); cg++){
			if (m_glyphs[cg]->textureID != m_glyphs[cg - 1]->textureID)
			{
				batchRenderers.emplace_back(offset, 6, m_glyphs[cg]->textureID);
			}
			else {
				batchRenderers.back().nVertices += 6;
			}

			vertices[cv++] = m_glyphs[cg]->topleft;
			vertices[cv++] = m_glyphs[cg]->bottomleft;
			vertices[cv++] = m_glyphs[cg]->bottomright;
			vertices[cv++] = m_glyphs[cg]->bottomright;
			vertices[cv++] = m_glyphs[cg]->topright;
			vertices[cv++] = m_glyphs[cg]->topleft;
			offset += 6;
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_vbo[0]);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), nullptr, glMODE);
		glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(Vertex), vertices.data());
		glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

	void Sprite2DShader::Sprite2DBatch::sortGlyphs(){
		switch (m_sort){

		case SORT_TYPE::BY_TEXTURE_ONLY:
			std::stable_sort(m_glyphs.begin(), m_glyphs.end(), compare_Texture);
			break;
		case SORT_TYPE::BY_DEPTH_ONLY:
			std::stable_sort(m_glyphs.begin(), m_glyphs.end(), compare_Depth);
			break;
		case SORT_TYPE::BY_INVDEPTH_ONLY:
			std::stable_sort(m_glyphs.begin(), m_glyphs.end(), compare_InvDepth);
			break;
		case SORT_TYPE::BY_TEXTURE_DEPTH:
			std::stable_sort(m_glyphs.begin(), m_glyphs.end(), compare_TextureDepth);
			break;
		case SORT_TYPE::BY_DEPTH_TEXTURE:
			std::stable_sort(m_glyphs.begin(), m_glyphs.end(), compare_DepthTexture);
			break;

		default:
			break;
		}
	}

	bool Sprite2DShader::Sprite2DBatch::compare_Texture(Glyph* a, Glyph* b){
		return a->textureID < b->textureID;
	}

	bool Sprite2DShader::Sprite2DBatch::compare_Depth(Glyph* a, Glyph* b){
		return a->depth < b->depth;
	}

	bool Sprite2DShader::Sprite2DBatch::compare_InvDepth(Glyph* a, Glyph* b){
		return a->depth > b->depth;
	}

	bool Sprite2DShader::Sprite2DBatch::compare_TextureDepth(Glyph* a, Glyph* b){
		return a->textureID <= b->textureID && a->depth < b->depth;
	}

	bool Sprite2DShader::Sprite2DBatch::compare_DepthTexture(Glyph* a, Glyph* b){
		return a->depth <= b->depth && a->textureID < b->textureID;
	}



}