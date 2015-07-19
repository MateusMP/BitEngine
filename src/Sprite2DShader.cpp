#include "Sprite2DShader.h"


#define GLSL(version, shader)  "#version " #version "\n" #shader

// NO TRANSFORM_SHADER
/*
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
*/

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* sprite2DshaderVertex_transform = GLSL(150,
	in vec2 a_vertexPosition;
	in vec2 a_textureCoord;
	in mat3 a_modelMatrix;

	out vec2 fragTextureCoord;

	uniform mat4 u_viewMatrix;

	void main()
	{
		vec3 vertexWorldPos = a_modelMatrix * vec3(a_vertexPosition, 1.0f);
		gl_Position.xy = (u_viewMatrix * vec4(vertexWorldPos.xy, 0, 1.0f)).xy;
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
		return CompileShadersSources(sprite2DshaderVertex_transform, sprite2DshaderFragment);
	}

	void Sprite2DShader::LoadViewMatrix(const glm::mat4& matrix)
	{
		u_viewMatrix = matrix;
	}

	void Sprite2DShader::BindAttributes() {
		BindAttribute(ATTR_VERTEX_POS, "a_vertexPosition");
		BindAttribute(ATTR_VERTEX_TEX, "a_textureCoord");
		BindAttribute(ATTR_MODEL_MAT, "a_modelMatrix");

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
		
		// Attributes
		
		glBindBuffer(GL_ARRAY_BUFFER, outVBO[VBO_VERTEX]);
		glEnableVertexAttribArray(ATTR_VERTEX_POS);
		glVertexAttribPointer(ATTR_VERTEX_POS, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
		
		glEnableVertexAttribArray(ATTR_VERTEX_TEX);
		glVertexAttribPointer(ATTR_VERTEX_TEX, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, uv));

		glBindBuffer(GL_ARRAY_BUFFER, outVBO[VBO_MODELMAT]);
		for (int i = 0; i < VERTEX_MATRIX3_ATTIBUTE_SIZE; ++i){
			glEnableVertexAttribArray(ATTR_MODEL_MAT+i);
			glVertexAttribPointer(ATTR_MODEL_MAT + i, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(sizeof(GLfloat)*3*i) );
			glVertexAttribDivisor(ATTR_MODEL_MAT + i, 1);
		}

		glBindVertexArray(0);

		return vao;
	}



}