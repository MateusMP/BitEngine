#pragma once

#include "BitEngine/Core/Sprite2DShader.h"

#include "Platform/opengl/ShaderProgram.h"
#include "Platform/opengl/VertexArrayObject.h"

// Shader Implementation for Sprite2D supporting GL3 and GL4

// Vertex Shader - Instancing
static const char* Sprite2D_vertex_GL3_GL4 = GLSL_(150,
	in vec2 a_textureCoord[4];
	in vec4 a_offset;
	in mat3 a_modelMatrix;

	out vec2 fragTextureCoord;

	uniform mat4 u_viewMatrix;

	const vec3 vertex_pos[4] = vec3[4](
		vec3(0.0f, 0.0f, 1.0f),
		vec3(1.0f, 0.0f, 1.0f),
		vec3(0.0f, 1.0f, 1.0f),
		vec3(1.0f, 1.0f, 1.0f)
	);

	void main()
	{
		vec2 vertexPos = vertex_pos[gl_VertexID].xy * a_offset.zw;

		vec3 vertexWorldPos = a_modelMatrix * vec3(vertexPos + a_offset.xy*a_offset.zw, 1);
		gl_Position.xy = (u_viewMatrix * vec4(vertexWorldPos.xy, 0, 1.0f)).xy;
		gl_Position.z = 0.0;
		gl_Position.w = 1.0;

		fragTextureCoord = a_textureCoord[gl_VertexID];
	}
);

namespace BitEngine{

	// Creating Vertex Data
	DECLARE_VERTEXDATA(Sprite2Dinstanced_VDVertices, 5)
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 1)
		END_ATTRIBUTES()

		DECLARE_DATA_STRUCTURE()
		ADD_MEMBER_ARRAY(glm::vec2, tex_coord, 4)
		ADD_MEMBER(glm::vec4, size_offset)
	END_VERTEX_DATA()

	DECLARE_VERTEXDATA(Sprite2Dinstanced_VDModelMatrix, 3)
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
		ADD_ATTRIBUTE(GL_FLOAT, 3, GL_FALSE, 1),
		END_ATTRIBUTES()

		DECLARE_DATA_STRUCTURE()
		ADD_MEMBER(glm::mat3, modelMatrix)
	END_VERTEX_DATA()

	typedef VertexArrayObject<IVBO<Sprite2Dinstanced_VDVertices>, IVBO<Sprite2Dinstanced_VDModelMatrix>> Sprite2Dinstanced_VAOinterleaved;

	// Implementation for GL4 or GL3
	class Sprite2DGL4 : public Sprite2DShader, protected ShaderProgram
	{
	public:
		Sprite2DGL4(RendererVersion v) 
			: version(v) {}
		~Sprite2DGL4(){}
		
		void BindShader() override{
			this->ShaderProgram::Bind();
		}

		// Compile shaders
		int Init() override{
			// Check if functions are available
			if (version == USE_GL4){
				if (!BatchRenderer::CheckFunctionsGL4()){
					version = USE_GL3;
					LOG(EngineLog, BE_LOG_INFO) << "Functions for Sprite2D GL4 not available! Fallback to GL3... ";
				}
			}

			if (version == USE_GL3) {
				if (!BatchRenderer::CheckFunctionsGL3()){
					version = NOT_AVAILABLE;
					return SHADER_INIT_ERROR_NO_FUNCTIONS;
				}
			}

			// Try to compile the shaders
			int build = BuildProgramFromMemory(GL_VERTEX_SHADER, Sprite2D_vertex_GL3_GL4,
				GL_FRAGMENT_SHADER, Sprite2D_fragment_GLall);
			if (build == BE_NO_ERROR)
			{
				LOG(EngineLog, BE_LOG_INFO) << "Using Sprite2D " << ((version == USE_GL4) ? "GL4" : "GL3");
				return BE_NO_ERROR;
			}

			return SHADER_INIT_ERROR_NO_RENDERER;
		}

		void LoadViewMatrix(const glm::mat4& matrix) override{
			u_viewMatrix = matrix;
		}

		virtual Sprite2DBatchRenderer* CreateRenderer() const {
			return new BatchRenderer(version);
		}

	protected:
		void BindAttributes() override {
			BindAttribute(0, "a_textureCoord[0]");
			BindAttribute(1, "a_textureCoord[1]");
			BindAttribute(2, "a_textureCoord[2]");
			BindAttribute(3, "a_textureCoord[3]");
			BindAttribute(4, "a_offset");
			BindAttribute(5, "a_modelMatrix");
		}

		void RegisterUniforms() override {
			LOAD_UNIFORM(u_texDiffuseHdl, "u_texDiffuse");
			LOAD_UNIFORM(u_viewMatrixHdl, "u_viewMatrix");
		}

		void OnBind() override{
			connectTexture(u_texDiffuseHdl, TEXTURE_DIFFUSE);

			loadMatrix4f(u_viewMatrixHdl, &(u_viewMatrix[0][0]));
		}

	protected:
		// Locations
		s32 u_texDiffuseHdl;
		s32 u_viewMatrixHdl;

	private:
		RendererVersion version;

		// Uniform data
		glm::mat4 u_viewMatrix;

	public:
		void* operator new(size_t size){ return _aligned_malloc(size, 16); }
		void operator delete(void* mem) { return _aligned_free(mem); }

	private:
		// Works for GL3 or GL4
		class BatchRenderer : public Sprite2DBatchRenderer
		{
		public:
			static bool CheckFunctionsGL4(){
				if (glDrawArraysInstancedBaseInstance == nullptr)
					return false;

				return true;
			}
			static bool CheckFunctionsGL3(){
				if (glDrawArraysInstanced == nullptr)
					return false;

				return true;
			}

			BatchRenderer(RendererVersion version)
				: RENDERER_VERSION(version)
			{
				{
					m_sortActive = true;

					// Make sure we have the vao needed
					m_interVAOs.emplace_back();
					m_interVAOs.back().Create();
				}
			}

			~BatchRenderer()
			{
				for (auto& v : m_interVAOs){
					v.Destroy();
				}
			}


			void Begin() override
			{
				m_elements.clear();
			}

			void End() override
			{
				if (m_elements.empty())
					return;

				sortComponents();
				createRenderers();
			}

			void setSorting(bool sort) override { m_sortActive = sort; }

			void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth) override
			{
				m_elements.emplace_back(depth, sprite, modelMatrix);
			}

			void Render() override
			{
				if (m_elements.empty())
					return;

				renderBatches();
			}

		public:
			struct Batch{
				Batch(u32 _offset, int _nI, u32 _texture, bool tr)
					: offset(_offset), nItems(_nI), texture(_texture), transparent(tr)
				{}

				u32 offset;
				int nItems;
				u32 texture;
				bool transparent;

				GLuint iVAO;
				GLuint* iVBO;
			};

		private:

			void sortComponents()
			{
				if (m_sortActive)
				{
					std::stable_sort(m_elements.begin(), m_elements.end(), compare_DepthTexture);
				}
			}

			void createRenderers()
			{
				createRenderersGL4GL3();
			}

			void renderBatches()
			{
				glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);

				if (RENDERER_VERSION == USE_GL4){
					renderGL4();
				}
				else if (RENDERER_VERSION == USE_GL3){
					renderGL3();
				}
			}

			void createRenderersGL4GL3()
			{
				BufferVector<Sprite2Dinstanced_VDVertices> vertices_;
				BufferVector<Sprite2Dinstanced_VDModelMatrix> modelMatrices_;

				vertices_.resize(m_elements.size());
				modelMatrices_.resize(m_elements.size());

				batches.clear();

				int offset = 0;
				const Sprite* lastSpr = nullptr;
				for (u32 cg = 0; cg < m_elements.size(); cg++)
				{
					const Sprite* spr = m_elements[cg].sprite;
					const glm::vec4& uvrect = spr->getUV();

					if (spr != lastSpr
						|| spr->getTexture() != lastSpr->getTexture()
						|| spr->isTransparent() != lastSpr->isTransparent())
					{
						//batches.emplace_back(offset, 0, spr->getTexture()->getTextureID(), spr->isTransparent());
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
				if (RENDERER_VERSION == USE_GL4)
				{
					m_interVAOs[0].IVBO<Sprite2Dinstanced_VDVertices>::BindAndLoadBuffer(vertices_);
					m_interVAOs[0].IVBO<Sprite2Dinstanced_VDModelMatrix>::BindAndLoadBuffer(modelMatrices_);

					VertexArrayHelper::UnbindVBO();
				}
				else if (RENDERER_VERSION == USE_GL3)
				{
					// Create more VAOs if needed
					while (m_interVAOs.size() < batches.size()){
						m_interVAOs.emplace_back();
						m_interVAOs.back().Create();
					}

					// Bind data for each batch
					for (u32 i = 0; i < batches.size(); ++i)
					{
						Batch& b = batches[i];

						m_interVAOs[i].IVBO<Sprite2Dinstanced_VDVertices>::BindAndLoadBufferRange(vertices_, b.offset, b.nItems);
						m_interVAOs[i].IVBO<Sprite2Dinstanced_VDModelMatrix>::BindAndLoadBufferRange(modelMatrices_, b.offset, b.nItems);
					}
					VertexArrayHelper::UnbindVBO();
				}
			}

			void renderGL4()
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

				VertexArrayHelper::UnbindVAO();
			}

			void renderGL3()
			{
				for (u32 i = 0; i < batches.size(); ++i)
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

				VertexArrayHelper::UnbindVAO();
			}

		private:
			RendererVersion RENDERER_VERSION;
			bool m_sortActive;

			std::vector<RenderingElement> m_elements;

			// Batches
			std::vector<Batch> batches;

			std::vector<Sprite2Dinstanced_VAOinterleaved> m_interVAOs; // GL3 and GL4
		};

	};
}