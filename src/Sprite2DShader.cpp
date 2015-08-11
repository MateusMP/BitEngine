#include "Sprite2DShader.h"


#define GLSL(version, shader)  "#version " #version "\n" shader
#define GLSL_(version, shader)  "#version " #version "\n" #shader

// Use this vertex for GL3 and GL4 -> Uses instancing
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

// Shader for GL2
// Does not uses instancing!
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


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static const char* Sprite2D_fragment_GLall = GLSL_(150,
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

	Sprite2DShader::RendererVersion Sprite2DShader::useRenderer = NOT_DEFINED;

	Sprite2DShader::RendererVersion Sprite2DShader::DetectBestRenderer()
	{
		if (useRenderer != NOT_DEFINED){
			return useRenderer;
		}

		if (glewIsSupported("GL_VERSION_4_2"))
		{
			useRenderer = RendererVersion::USE_GL4;
		}
		else if (glewIsSupported("GL_VERSION_3_0"))
		{
			useRenderer = RendererVersion::USE_GL3;
		}
		else
		{
			useRenderer = RendererVersion::USE_GL2;
		}

		return useRenderer;
	}

	Sprite2DShader::Sprite2DShader()
	{
	}


	Sprite2DShader::~Sprite2DShader()
	{
	}

	int Sprite2DShader::Init(RendererVersion forceVersion)
	{
		useRenderer = forceVersion;
		return Init();
	}

	int Sprite2DShader::Init()
	{
		// Try to guess the best renderer
		if (DetectBestRenderer() == NOT_DEFINED){
			return SHADER_INIT_ERROR_NO_RENDERER;
		}

		// Try to compile the shaders

		// GL3 and GL4 [instancing]
		if ( (useRenderer == USE_GL4 || useRenderer == USE_GL3) )
		{
			int build = BuildProgramFromMemory(GL_VERTEX_SHADER, Sprite2D_vertex_GL3_GL4,
											   GL_FRAGMENT_SHADER, Sprite2D_fragment_GLall);
			if (build == NO_ERROR)
			{
				LOGTO(Info) << "Using Sprite2D " << ((useRenderer==USE_GL4)?"GL4":"GL3") << endlog;
				return NO_ERROR;
			}
			
			// If failed to compile -> fallback to GL2
			LOGTO(Warning) << "Could not compile GL3/GL4 shader for Sprite2D, driver update needed? Fallback to GL2..." << endlog;
			useRenderer = USE_GL2;
		}

		// GL2
		if (useRenderer == USE_GL2)
		{
			int build = BuildProgramFromMemory(GL_VERTEX_SHADER, Sprite2D_vertex_GL2,
											   GL_FRAGMENT_SHADER, Sprite2D_fragment_GLall);
			if (build == NO_ERROR){
				LOGTO(Info) << "Using Sprite2D GL2!" << endlog;
				return NO_ERROR;
			}
		}


		LOGTO(Error) << "Could not initialize Sprite2D shader!" << endlog;
		return SHADER_INIT_ERROR_NO_RENDERER;
	}

	void Sprite2DShader::LoadViewMatrix(const glm::mat4& matrix)
	{
		u_viewMatrix = matrix;
	}

	void Sprite2DShader::BindAttributes() 
	{
		if (useRenderer == RendererVersion::USE_GL3 || useRenderer == RendererVersion::USE_GL4){
			BindAttribute(0, "a_textureCoord[0]");
			BindAttribute(1, "a_textureCoord[1]");
			BindAttribute(2, "a_textureCoord[2]");
			BindAttribute(3, "a_textureCoord[3]");
			BindAttribute(4, "a_offset");
			BindAttribute(5, "a_modelMatrix");
		}
		else {
			BindAttribute(0, "a_position");
			BindAttribute(1, "a_uvcoord");
		}

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