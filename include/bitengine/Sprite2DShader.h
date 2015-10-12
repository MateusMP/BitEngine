#pragma once

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "ShaderProgram.h"
#include "glSystem.h"

#include "VertexArrayObject.h"

#include "Sprite.h"

namespace BitEngine
{
	/// Draws 2D sprites using 2D coordinates
	/// 
	// GL3 and GL4
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

//	typedef VertexArrayObject
//		<SVBO<Sprite2Dinstanced_VDVertices>, SVBO<Sprite2Dinstanced_VDModelMatrix>> Sprite2Dinstanced_VAOseparated;
	typedef VertexArrayObject
		<IVBO<Sprite2Dinstanced_VDVertices>, IVBO<Sprite2Dinstanced_VDModelMatrix>> Sprite2Dinstanced_VAOinterleaved;

	// GL2
	DECLARE_VERTEXDATA(Sprite2Dbasic_VD, 2)
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 0),
		ADD_ATTRIBUTE(GL_FLOAT, 2, GL_FALSE, 0),
		END_ATTRIBUTES()

		DECLARE_DATA_STRUCTURE()
			ADD_MEMBER(glm::vec2, vertexPos)
			ADD_MEMBER(glm::vec2, vertexUV)
	END_VERTEX_DATA()

	typedef VertexArrayObject < IVBO<Sprite2Dbasic_VD> > Sprite2Dbasic_VAOinterleaved;

	class Sprite2DShader
	{
		public:
			static const uint32 TEXTURE_DIFFUSE = 0;
			
			static RendererVersion GetRendererVersion(){ return useRenderer; }

			class Sprite2DBatchRenderer
			{
				public:
					virtual ~Sprite2DBatchRenderer(){}

					virtual void Begin() = 0;
					virtual void End() = 0;

					virtual void setSorting(bool sort) = 0;

					virtual void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth) = 0;

					virtual void Render() = 0;

					struct RenderingElement {
						RenderingElement(GLuint _depth, const Sprite* _sprite, const glm::mat3* _modelMatrix)
							: depth(_depth), sprite(_sprite), modelMatrix(_modelMatrix)
						{}

						GLuint depth;
						const Sprite* sprite;
						const glm::mat3* modelMatrix;
					};

					static bool compare_DepthTexture(const RenderingElement& a, const RenderingElement& b){
						return a.depth < b.depth
							|| (a.depth == b.depth && (a.sprite->getTexture() < b.sprite->getTexture()));
					}
			};

		private:
			static RendererVersion useRenderer;

		public:
			Sprite2DShader();
			~Sprite2DShader();

			// Initialize shader for current best available GL version
			int Init();
			
			// Forces to given version (if not possible, will fallback to an older one)
			// This enables that newer computers execute the code path for older GL (testing purpouses)
			int Init(RendererVersion forceVersion);

			void Bind();

			void LoadViewMatrix(const glm::mat4& matrix);

			
			Sprite2DBatchRenderer* CreateRenderer()
			{
				return m_shader->CreateRenderer();
			}

		private:
			class IShaderVersion : public ShaderProgram{
			public:
				virtual ~IShaderVersion(){};
				virtual void LoadViewMatrix(const glm::mat4& matrix) = 0;
				virtual Sprite2DBatchRenderer* CreateRenderer() const = 0;
			};

			IShaderVersion* m_shader;

			// Implementation for GL4 or GL3
			class ShaderGL4 : public IShaderVersion
			{
				public:
					ShaderGL4(RendererVersion v);
					~ShaderGL4();

					// Compile shaders
					int Init() override;

					void LoadViewMatrix(const glm::mat4& matrix) override;

					virtual Sprite2DBatchRenderer* CreateRenderer() const{
						return new BatchRenderer(version);
					}

				protected:
					void BindAttributes() override;

					void RegisterUniforms() override;

					void OnBind() override;

				protected:
					// Locations
					int32 u_texDiffuseHdl;
					int32 u_viewMatrixHdl;

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

						BatchRenderer(RendererVersion version);
						~BatchRenderer();

						void Begin() override;
						void End() override;

						void setSorting(bool sort) override { m_sortActive = sort; }

						void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth) override;

						void Render() override;

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

						void sortComponents();
						void createRenderers();
						void renderBatches();

						void createRenderersGL4GL3();

						void renderGL4();
						void renderGL3();
						
					private:
						RendererVersion RENDERER_VERSION;
						bool m_sortActive;

						std::vector<RenderingElement> m_elements;

						// Batches
						std::vector<Batch> batches;

						std::vector<Sprite2Dinstanced_VAOinterleaved> m_interVAOs; // GL3 and GL4
					};

			};

			// Implementation for GL2
			class ShaderGL2 : public IShaderVersion
			{
			public:
				ShaderGL2();
				~ShaderGL2();

				// Compile shaders
				int Init() override;

				void LoadViewMatrix(const glm::mat4& matrix) override;

				virtual Sprite2DBatchRenderer* CreateRenderer() const{
					return new BatchRenderer();
				}

			protected:
				void BindAttributes() override;

				void RegisterUniforms() override;

				void OnBind() override;

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

			private:
				class BatchRenderer : public Sprite2DBatchRenderer
				{
				public:
					static bool CheckFunctions(){
						if (glBindVertexArray == nullptr)
							return false;

						return true;
					}

					BatchRenderer();
					~BatchRenderer();

					void Begin() override;
					void End() override;

					void setSorting(bool sort) override { m_sortActive = sort; }

					void DrawSprite(const Sprite* sprite, const glm::mat3* modelMatrix, int depth) override;

					void Render() override;

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

					void sortComponents();
					void createRenderers();
					void renderBatches();

				private:
					RendererVersion RENDERER_VERSION;
					bool m_sortActive;

					std::vector<RenderingElement> m_elements;

					// Batches
					std::vector<Batch> batches;

					std::vector<Sprite2Dbasic_VAOinterleaved> m_basicVAOs; // GL2
				};

			};
	};


}