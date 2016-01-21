#pragma once

#include "Core/Sprite2DShader.h"

#include "DefaultBackends/opengl/ShaderProgram.h"
#include "DefaultBackends/opengl/VertexArrayObject.h"
#include "DefaultBackends/opengl/glSystem.h"

// Shader Implementation for Sprite2D supporting GL2

// Vertex Shader - Without instancing
static const char* Sprite2D_vertex_GL2 = GLSL_(120,
	attribute vec2 a_position;
	attribute vec2 a_uvcoord;

	varying vec2 fragTextureCoord;

	uniform mat4 u_viewMatrix;

	void main()
	{
		gl_Position.xy = (u_viewMatrix * vec4(a_position, 0, 1.0f)).xy;
		gl_Position.z = 0.0;
		gl_Position.w = 1.0;

		fragTextureCoord = a_uvcoord;
	}
);

namespace BitEngine{

	// Creating Vertex Data
	DECLARE_VERTEXDATA(Sprite2Dbasic_VD, 2)
	ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 0),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 0),
		END_ATTRIBUTES()

		DECLARE_DATA_STRUCTURE()
	ADD_MEMBER(glm::vec2, vertexPos)
		ADD_MEMBER(glm::vec2, vertexUV)
		END_VERTEX_DATA()

	typedef VertexArrayObject < IVBO<Sprite2Dbasic_VD> > Sprite2Dbasic_VAOinterleaved;

	// Implementation for GL2
	class Sprite2DGL2 : public Sprite2DShader, protected ShaderProgram
	{
	public:
		Sprite2DGL2(){}
		~Sprite2DGL2(){}

		void BindShader() override {
			this->ShaderProgram::Bind();
		}

		// Compile shaders
		int Init() override {
			// Try to compile the shaders
			if (!BatchRenderer::CheckFunctions())
				return SHADER_INIT_ERROR_NO_FUNCTIONS;

			int build = BuildProgramFromMemory(GL_VERTEX_SHADER, Sprite2D_vertex_GL2,
				GL_FRAGMENT_SHADER, Sprite2D_fragment_GLall);
			if (build == NO_ERROR){
				LOGTO(Info) << "Using Sprite2D GL2!" << endlog;
				return NO_ERROR;
			}

			return SHADER_INIT_ERROR_NO_RENDERER;
		}

		void LoadViewMatrix(const glm::mat4& matrix) override{
			u_viewMatrix = matrix;
		}

		Sprite2DBatchRenderer* CreateRenderer() const override {
			return new BatchRenderer();
		}

	protected:
		void BindAttributes() override {
			BindAttribute(0, "a_position");
			BindAttribute(1, "a_uvcoord");

			check_gl_error();
		}

		void RegisterUniforms() override {
			LOAD_UNIFORM(u_texDiffuseHdl, "u_texDiffuse");
			LOAD_UNIFORM(u_viewMatrixHdl, "u_viewMatrix");
		}

		void OnBind() override {
			connectTexture(u_texDiffuseHdl, TEXTURE_DIFFUSE);

			loadMatrix4f(u_viewMatrixHdl, &(u_viewMatrix[0][0]));
		}

	protected:
		// Locations
		int32 u_texDiffuseHdl;
		int32 u_viewMatrixHdl;

	private:
		// Uniform data
		glm::mat4 u_viewMatrix;

	public:
		void* operator new(size_t size){ return _aligned_malloc(size, 16); }
		void operator delete(void* mem) { return _aligned_free(mem); }
		
	private:
		class BatchRenderer : public Sprite2DShader::Sprite2DBatchRenderer
		{
		public:
			static bool CheckFunctions(){
				if (glBindVertexArray == nullptr)
					return false;

				return true;
			}

			BatchRenderer() : m_sortActive(true){}

			~BatchRenderer()
			{
				for (auto& x : m_basicVAOs)
					x.Destroy();
			}

			void Begin() override {
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

			void DrawSprite(const BitEngine::Sprite* sprite, const glm::mat3* modelMatrix, int depth) override {
				m_elements.emplace_back(depth, sprite, modelMatrix);
			}

			void Render() override {
				if (m_elements.empty())
					return;

				renderBatches();
			}

		public:
			struct Batch{
				Batch(uint32 _offset, int _nI, uint32 _texture, bool tr)
					: offset(_offset), nItems(_nI), texture(_texture), transparent(tr)
				{}

				uint32 offset;
				int nItems;
				uint32 texture;
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
					vertices_[vertexID + 0].vertexUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw
					vertices_[vertexID + 1].vertexUV = glm::vec2(uvrect.z, uvrect.y); // BR  
					vertices_[vertexID + 2].vertexUV = glm::vec2(uvrect.x, uvrect.w); // TL  
					vertices_[vertexID + 3].vertexUV = glm::vec2(uvrect.x, uvrect.w); // TL  
					vertices_[vertexID + 4].vertexUV = glm::vec2(uvrect.z, uvrect.y); // BR  
					vertices_[vertexID + 5].vertexUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy
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

				check_gl_error();
			}

			void renderBatches()
			{
				glActiveTexture(GL_TEXTURE0 + Sprite2DShader::TEXTURE_DIFFUSE);

				// Render all batches
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

				check_gl_error();
			}

		private:
			bool m_sortActive;

			std::vector<RenderingElement> m_elements;

			// Batches
			std::vector<Batch> batches;

			std::vector<Sprite2Dbasic_VAOinterleaved> m_basicVAOs; // GL2
		};

	};


}