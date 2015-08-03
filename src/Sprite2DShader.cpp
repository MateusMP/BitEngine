#include "Sprite2DShader.h"


#define GLSL(version, shader)  "#version " #version "\n" shader
#define GLSL_(version, shader)  "#version " #version "\n" #shader

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

static const char* sprite2DshaderVertex_transform = GLSL_(150,
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

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* sprite2DshaderFragment = GLSL_(150,
	in vec2 fragTextureCoord;

	out vec4 finalColor;

	uniform sampler2D u_texDiffuse;

	void main()
	{
		finalColor = texture(u_texDiffuse, fragTextureCoord);
	}
);
// 
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
namespace BitEngine{

	Sprite2DShader::Sprite2DShader()
	{
	}


	Sprite2DShader::~Sprite2DShader()
	{
	}

	int Sprite2DShader::Init(){
		return BuildProgramFromMemory(GL_VERTEX_SHADER, sprite2DshaderVertex_transform,
									  GL_FRAGMENT_SHADER, sprite2DshaderFragment);
	}

	void Sprite2DShader::LoadViewMatrix(const glm::mat4& matrix)
	{
		u_viewMatrix = matrix;
	}

	void Sprite2DShader::BindAttributes() {
		BindAttribute(ATTR_VERTEX_TEX+0, "a_textureCoord[0]");
		BindAttribute(ATTR_VERTEX_TEX+1, "a_textureCoord[1]");
		BindAttribute(ATTR_VERTEX_TEX+2, "a_textureCoord[2]");
		BindAttribute(ATTR_VERTEX_TEX+3, "a_textureCoord[3]");
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

	Sprite2DShader::Vao Sprite2DShader::CreateVAO() {
		Vao vaoHolder;

		// VAO
		glGenVertexArrays(1, &vaoHolder.VAO);
		if (vaoHolder.VAO == 0){
			LOGTO(Error) << "Sprite2DBatch: Could not create VAO." << endlog;
		}
		glBindVertexArray(vaoHolder.VAO);

		// VBO
		glGenBuffers(NUM_VBOS, vaoHolder.VBO);
		if (vaoHolder.VBO[0] == 0){
			LOGTO(Error) << "Sprite2DBatch: Could not create VBO." << endlog;
		}
		
		// Attributes
		glBindBuffer(GL_ARRAY_BUFFER, vaoHolder.VBO[VBO_VERTEXDATA]);
		for (int i = 0; i < ATTR_MODEL_MAT; ++i){
			glEnableVertexAttribArray(ATTR_VERTEX_TEX + i);
			glVertexAttribPointer(ATTR_VERTEX_TEX + i, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(i * 2 * sizeof(GLfloat)));
			glVertexAttribDivisor(ATTR_VERTEX_TEX + i, 1);
		}

		glEnableVertexAttribArray(ATTR_SPRITE_OFF);
		glVertexAttribPointer(ATTR_SPRITE_OFF, 4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, (void*)(8 * sizeof(GLfloat)));
		glVertexAttribDivisor(ATTR_SPRITE_OFF, 1);

		// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		// printf("offsetA: %d\n", offsetof(Vertex, position));
		// printf("offsetB: %d\n", offsetof(Vertex, uv));

		glBindBuffer(GL_ARRAY_BUFFER, vaoHolder.VBO[VBO_MODELMAT]);
		for (int i = 0; i < VERTEX_MATRIX3_ATTIBUTE_SIZE; ++i){
			glEnableVertexAttribArray(ATTR_MODEL_MAT + i);
			glVertexAttribPointer(ATTR_MODEL_MAT + i, 3, GL_FLOAT, GL_FALSE, sizeof(glm::mat3), (void*)(sizeof(GLfloat)*3*i) );
			glVertexAttribDivisor(ATTR_MODEL_MAT + i, 1);
		}

		glBindVertexArray(0);

		return vaoHolder;
	}
}