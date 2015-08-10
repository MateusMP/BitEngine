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

	Sprite2DShader::Renderers Sprite2DShader::useRenderer = NOT_DEFINED;

	Sprite2DShader::Renderers Sprite2DShader::DetectBestRenderer()
	{
		if (useRenderer != NOT_DEFINED){
			return useRenderer;
		}

		if (glewIsSupported("GL_VERSION_4_2"))
		{
			useRenderer = Renderers::USE_GL4;
		}
		else if (glewIsSupported("GL_VERSION_3_0"))
		{
			useRenderer = Renderers::USE_GL3;
		}
		else
		{
			useRenderer = Renderers::USE_GL2;
		}

		return useRenderer;
	}

	Sprite2DShader::Sprite2DShader()
	{
	}


	Sprite2DShader::~Sprite2DShader()
	{
	}

	int Sprite2DShader::Init()
	{
		if (DetectBestRenderer() == NOT_DEFINED)
			return SHADER_INIT_ERROR_NO_RENDERER;

		return BuildProgramFromMemory(GL_VERTEX_SHADER, sprite2DshaderVertex_transform,
									  GL_FRAGMENT_SHADER, sprite2DshaderFragment);
	}

	void Sprite2DShader::LoadViewMatrix(const glm::mat4& matrix)
	{
		u_viewMatrix = matrix;
	}

	void Sprite2DShader::BindAttributes() 
	{
		BindAttribute(0, "a_textureCoord[0]");
		BindAttribute(1, "a_textureCoord[1]");
		BindAttribute(2, "a_textureCoord[2]");
		BindAttribute(3, "a_textureCoord[3]");
		BindAttribute(4, "a_offset");
		BindAttribute(5, "a_modelMatrix");

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
}