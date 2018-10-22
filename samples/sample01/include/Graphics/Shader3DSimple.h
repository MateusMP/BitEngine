#pragma once

#include <map>

#include <assimp/scene.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include <glm/common.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <bitengine/bitengine.h>
#include <bitengine/Core/Logger.h>
#include <bitengine/Platform/opengl/OpenGL.h>
#include <bitengine/Platform/opengl/VertexArrayObject.h>
#include <bitengine/Platform/opengl/ShaderProgram.h>


// GL4/3
DECLARE_VERTEXDATA(Simple3Dmatrix_VD, 4)
	ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
	ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
	ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0),
	ADD_ATTRIBUTE(GL_FLOAT, 4, GL_FALSE, 0)
	END_ATTRIBUTES()

	DECLARE_DATA_STRUCTURE()
		ADD_MEMBER(glm::mat4, matrix)
END_VERTEX_DATA()

class Shader3DSimple
{

public:
	class BatchRenderer;

	static const int DIFFUSE_TEXTURE_SLOT = 0;
	static const int NORMAL_TEXTURE_SLOT = 1;

	static const u32 ATTR_VERTEX_POS = 0;	// 3 floats	by vertex
	static const u32 ATTR_VERTEX_TEX = 1;	// 2 floats	by vertex
	static const u32 ATTR_VERTEX_NORMAL = 2;	// 3 floats by vertex
	static const u32 ATTR_MODEL_MAT = 3;		// 16 floats by instance

	static const u32 NUM_VBOS = 3;
	static const u32 VBO_INDEX = 0;
	static const u32 VBO_VERTEXDATA = 1;
	static const u32 VBO_MODELMAT = 2;

//	static RendererVersion GetRendererVersion(){ return useRenderer; }

	//

	Shader3DSimple();
	~Shader3DSimple();

	static int GetID(){
		return 1;
	}

	int Init();

	void LoadViewMatrix(const glm::mat4& matrix);
	void LoadProjectionMatrix(const glm::mat4& matrix);

	void Bind();

	BatchRenderer* CreateRenderer();

private:
	//static RendererVersion useRenderer;

	class IShaderVersion : public BitEngine::ShaderProgram{
	public:
		virtual void LoadViewMatrix(const glm::mat4& matrix) = 0;
		virtual void LoadProjectionMatrix(const glm::mat4& matrix) = 0;
		virtual BatchRenderer* CreateRenderer() = 0;
	};

	IShaderVersion* shader;

public:
	// Shader classes
	class Material : public BitEngine::Material
	{
	public:
		const BitEngine::Texture* diffuse;
		const BitEngine::Texture* normal;
	};

	class Vertex
	{
	public:
		Vertex(const glm::vec3& _pos,
			const glm::vec2& _uv,
			const glm::vec3& _normals)
			: pos(_pos), uv(_uv), normal(_normals){}

		glm::vec3 pos;
		glm::vec2 uv;
		glm::vec3 normal;
	};

	class Mesh : public BitEngine::Mesh
	{
	public:
		Mesh(const std::string& name,
			const std::vector<Vertex>& vertex,
			const std::vector<GLuint>& indices,
			Material* material)
			: BitEngine::Mesh(name), m_material(material)
		{
			LOG(BitEngine::EngineLog, BE_LOG_INFO) << "Creating VAO and VBOS: v: " << vertex.size() << " idx: " << indices.size();
			// Create buffers
			glGenVertexArrays(1, &vao);
			glGenBuffers(NUM_VBOS, vbo);

			// Load index buffer
			numIndices = indices.size();
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[VBO_INDEX]);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices*sizeof(GLuint), &indices[0], GL_STATIC_DRAW);

			// Vertex data
			glBindVertexArray(vao);
			glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_VERTEXDATA]);
			glEnableVertexAttribArray(ATTR_VERTEX_POS);
			glVertexAttribPointer(ATTR_VERTEX_POS, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)0);
			glVertexAttribDivisor(ATTR_VERTEX_POS, 0);

			glEnableVertexAttribArray(ATTR_VERTEX_TEX);
			glVertexAttribPointer(ATTR_VERTEX_TEX, 2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(3 * sizeof(GLfloat)));
			glVertexAttribDivisor(ATTR_VERTEX_TEX, 0);

			glEnableVertexAttribArray(ATTR_VERTEX_NORMAL);
			glVertexAttribPointer(ATTR_VERTEX_NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 8, (void*)(5 * sizeof(GLfloat)));
			glVertexAttribDivisor(ATTR_VERTEX_NORMAL, 0);

			// Load vertex data
			glBufferData(GL_ARRAY_BUFFER, vertex.size() * sizeof(Vertex), &vertex[0], GL_STATIC_DRAW);

			// Matrix
			//if (useRenderer != USE_GL2)
			{
				// Used only by GL4 renderer version
				glBindBuffer(GL_ARRAY_BUFFER, vbo[VBO_MODELMAT]);
				for (int i = 0; i < VERTEX_MATRIX4_ATTIBUTE_SIZE; ++i){
					glEnableVertexAttribArray(ATTR_MODEL_MAT + i);
					glVertexAttribPointer(ATTR_MODEL_MAT + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(GLfloat) * 4 * i));
					glVertexAttribDivisor(ATTR_MODEL_MAT + i, 1);
				}
			}

			glBindVertexArray(0);

			//check_gl_error();
		}

		~Mesh(){
			glDeleteBuffers(NUM_VBOS, vbo);
			glDeleteVertexArrays(1, &vao);
		}

		int getType() const override{
			return GetID();
		}

		Material* getMaterial() const override{
			return m_material;
		}

		const GLuint* getVBO() const{
			return vbo;
		}

		GLuint getVAO() const{
			return vao;
		}

		u32 getNumIndices() const{
			return numIndices;
		}

	private:
		Material *m_material;
		GLuint vao;
		GLuint vbo[NUM_VBOS];

		u32 numIndices;
	};

	class Model : public BitEngine::Model
	{
	public:
		Model(const std::string& baseDirectory);
		~Model();

		// Virtuals
		const std::string& getName() const override { return m_baseDir;  }
		const BitEngine::Material* getMaterial(int index) const { return materials[index]; }
		const BitEngine::Mesh* getMesh(int index) const { return meshes[index]; }
		//

		void process(BitEngine::ResourceLoader* tMng, const aiScene* scene);

	private:
		void processNode(aiNode* node, const aiScene* scene);
		typename Shader3DSimple::Mesh* processMesh(aiMesh* mesh, const aiScene* scene);
		Material* createMaterial(BitEngine::ResourceLoader* tMng, const aiMaterial* material);


		std::vector<Mesh*> meshes;
		std::vector<Material*> materials;

		std::string m_baseDir;
	};

	class BatchRenderer
	{
	public:
		virtual ~BatchRenderer(){}

		virtual void addMesh(Mesh* mesh, const Material* mat, const glm::mat4* modelMat) = 0;

		virtual void Begin() = 0;

		virtual void End() = 0;

		virtual void Render() = 0;

	protected:
		struct RenderData
		{
			RenderData(const Material* _mat, const glm::mat4* _model)
				: material(_mat), modelMatrix(_model) {}

			const Material* material;
			const glm::mat4* modelMatrix;
		};

		struct Batch{
			Batch(u32 _offset, int _nI, const Mesh* _mesh, const Material* _mat, bool tr)
				: offset(_offset), nItems(_nI), mesh(_mesh), material(_mat), transparent(tr)
			{}

			u32 offset;
			int nItems;
			const Mesh* mesh;
			const Material* material;
			bool transparent;
		};

		static bool sortRenderData(const RenderData& a, const RenderData& b)
		{
			return (a.material->diffuse < b.material->diffuse && a.material->normal < b.material->normal)
				|| (a.material->diffuse == b.material->diffuse && a.material->normal < b.material->normal);
		}
	};

	// Load model
	static Shader3DSimple::Model* LoadModel(BitEngine::ResourceLoader* tMng, const std::string& filename)
	{
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(filename, aiProcess_Triangulate);

		if (!scene || scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "ERROR::ASSIMP::" << importer.GetErrorString();
			return nullptr;
		}

		size_t pos = filename.find_last_of("/\\");
		std::string baseDir = filename.substr(0, pos) + "/";

		Model * m = new Model(baseDir);
		m->process(tMng, scene);

		return m;
	}

private:
	class Shader3DSimpleGL4 : public IShaderVersion
	{
	public:

		Shader3DSimple::BatchRenderer* CreateRenderer();

		void BindAttributes() override;

		/// VIRTUAL
		void RegisterUniforms() override;

		/// Virtual
		void OnBind() override;

		int Init() override;

		void LoadViewMatrix(const glm::mat4& matrix);
		void LoadProjectionMatrix(const glm::mat4& matrix);

	public:
		class BatchRendererGL4 : public Shader3DSimple::BatchRenderer
		{
		public:
			static bool CheckFunctions()
			{
				if (glDrawElementsInstancedBaseInstance == nullptr)
					return false;

				return true;
			}

			BatchRendererGL4();
			~BatchRendererGL4();

			void addMesh(Mesh* mesh, const Material* mat, const glm::mat4* modelMat) override;

			void Begin() override;

			void End() override;

			void Render() override;

		private:
			//
			void sort();

			void createBatches();

		private:
			std::map<Mesh*, std::vector<RenderData>> data;

			std::vector<Batch> batches;

			GLuint modelMatrixVBO;
		};


	protected:
		// Locations
		s32 u_projectionMatrixHDL;
		s32 u_viewMatrixHDL;
		s32 u_diffuseHDL;
		s32 u_normalHDL;

		// Data
		glm::mat4 u_projection;
		glm::mat4 u_view;


	public:
		void* operator new(size_t size){ return _aligned_malloc(size, 16); }
		void operator delete(void* mem) { return _aligned_free(mem); }

	};

	class Shader3DSimpleGL2 : public IShaderVersion
	{
	public:
		Shader3DSimple::BatchRenderer* CreateRenderer();

		void BindAttributes() override;

		/// VIRTUAL
		void RegisterUniforms() override;

		/// Virtual
		void OnBind() override;

		int Init() override;

		void LoadViewMatrix(const glm::mat4& matrix);
		void LoadProjectionMatrix(const glm::mat4& matrix);

		void LoadIntanceModelMatrix(const glm::mat4& matrix);

	public:
		class BatchRendererGL2 : public BatchRenderer
		{
		public:
			static bool CheckFunctions()
			{
				if (glBindVertexArray == nullptr)
					return false;

				return true;
			}

			BatchRendererGL2(Shader3DSimpleGL2* shader);

			~BatchRendererGL2();

			void addMesh(Mesh* mesh, const Material* mat, const glm::mat4* modelMat) override;

			void Begin() override;

			void End() override;

			void Render() override;

		private:
			//
			void sort();

			void createBatches();

		private:
			std::map<Mesh*, std::vector<RenderData>> data;

			std::vector<Batch> batches;

			std::vector<glm::mat4> modelMatrices;
			Shader3DSimpleGL2* m_shader;
		};


	protected:
		// Locations
		s32 u_projectionMatrixHDL;
		s32 u_viewMatrixHDL;
		s32 u_diffuseHDL;
		s32 u_normalHDL;
		s32 u_modelMatrixHDL;

		// Data
		glm::mat4 u_projection;
		glm::mat4 u_view;


	public:
		void* operator new(size_t size){ return _aligned_malloc(size, 16); }
		void operator delete(void* mem) { return _aligned_free(mem); }

	};

};
