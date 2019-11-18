#pragma once

#include <string>
#include <vector>

#include "OpenGL_Headers.h"

#include "BitEngine/Common/TypeDefinition.h"
#include "BitEngine/Common/ErrorCodes.h"
#include "BitEngine/Core/Logger.h"


namespace BitEngine
{

#define GLSL(version, shader)  "#version " #version "\n" shader
#define GLSL_(version, shader)  "#version " #version "\n" #shader



#ifdef _DEBUG
#define LOAD_UNIFORM(var, name)																						\
				var = getUniformLocation(name);																		\
				if (var < 0)																						\
					LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Shader: Failed to load "#var" [\""#name"\"] uniform.";	\
				else																								\
					LOG(BitEngine::EngineLog, BE_LOG_VERBOSE) << "Uniform "#var" loaded with id: "<< var << "."
#else
#define LOAD_UNIFORM(var, name)	\
					var = getUniformLocation(name)
#endif

#define ARRAY_OFFSET(x) ((void*)(x))

	// If using matrices, some macros to help define the correct attribute sizes
#define VERTEX_MATRIX3_ATTIBUTE_SIZE 3
#define VERTEX_MATRIX4_ATTIBUTE_SIZE 4

class ShaderProgram
{
    public:
        ShaderProgram();
        virtual ~ShaderProgram();

		/// Init the shader
		/// Normally calls BuildProgramFromFile/Memory
		virtual int Init() = 0;

		/// Binds the shader
		/// Calls OnBind()
        void Bind();

		/// Unbinds the shader
        void Unbind();

		struct Attribute {
			GLuint index;
			GLint location;
			GLenum type;
			GLint size;
			std::string name;
		};
		void introspect()
		{
			GLint nAttrs;
			glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES, &nAttrs);
			//m_attributes.resize(nAttrs);

			for (int i = 0; i < nAttrs; ++i)
			{
				GLint nameRead;
				Attribute attr;// = m_attributes[i];
				attr.name.resize(128);

				glGetActiveAttrib(m_programID, i, 128, &nameRead, &attr.size, &attr.type, &attr.name[0]);
				attr.name.resize(nameRead);
				attr.location = glGetAttribLocation(m_programID, attr.name.data());
				LOG(EngineLog, BE_LOG_INFO) << "Attr" << i << " at " << attr.location
					<< " '" << attr.name << "' is of type: " << attr.type << " size: " << attr.size;
			}

			GLint nUnif;
			glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &nUnif);
			for (int i = 0; i < nUnif; ++i)
			{
				GLint nameRead;
				Attribute unif;// = m_uniforms[i];
				unif.name.resize(128);

				glGetActiveUniform(m_programID, i, 128, &nameRead, &unif.size, &unif.type, &unif.name[0]);
				unif.name.resize(nameRead);
				unif.location = glGetUniformLocation(m_programID, unif.name.data());
				LOG(EngineLog, BE_LOG_INFO) << "Unif" << i << " at " << unif.location
					<< " '" << unif.name << "' is of type: " << unif.type << " size: " << unif.size;
			}
		}

	protected:
        GLuint m_programID; //!< program unique id

		// BUILDING THE SHADER PROGRAM

		/// \brief Build and link a set of sources from file to create a final Shader Program
		/// \return Returns BE_NO_ERROR in case of success
		template <typename ...Args>
		int BuildProgramFromFile(GLint type, const std::string& file, Args... args){
			std::vector<GLuint> shaders;

			if (CompileFromFile(shaders, type, file, args...) == BE_NO_ERROR){
				return BuildFinalProgram(shaders);
			}

			return FAILED_TO_COMPILE;
		}

		/// \brief Build and link a set of sources from memory to create a final Shader Program
		/// \return Returns BE_NO_ERROR in case of success
		template <typename ...Args>
		int BuildProgramFromMemory(GLint type, const char* source, Args... args){
			std::vector<GLuint> shaders;

			if (CompileFromMemory(shaders, type, source, args...) == BE_NO_ERROR){
				return BuildFinalProgram(shaders);
			}

			return FAILED_TO_COMPILE;
		}


		// ATTRIBUTE/UNIFORM FUNCTIONS

		/// Bind attribute ID to specified variable name on shader
		/// Usually called inside BindAttributes implementation
		void BindAttribute(int attrid, const std::string& name);

		/// Get uniform location on shader
		/// Usually called inside RegisterUniforms implementation
		s32 getUniformLocation(const std::string& name) const;


		// Loading uniform data functions
		// They are normally used on OnBind() implmentation
		// or between draw calls for changing drawing parameters

		/// Load a single INT uniform
		void loadInt(int location, int value);

		/// Load a single FLOAT uniform
		void loadFloat(int location, float value);

		/// Load 3 floats (vec3)
		void loadVector3f(int location, int n, const float* vector);

		/// Load 4 floats (vec4)
		void loadVector4f(int location, int n, const float* vector);

		/// Load a single Bool
		void loadBoolean(int location, bool b);

		/// Loads a single float Matrix 4x4
		void loadMatrix4f(int location, const float* matrix);

		/// \param unitID GL flag to indicate texture unit (GL_TEXTURE0 ... )
		void connectTexture(int location, int unitID);

	private:
		/// VIRTUAL
		// \param outVBO vector to store all VBO's id
		// Returns a VAO with all ShaderConfigurations Set
		// GLuint CreateVAO(GLuint* outVBO) = 0;

		/// VIRTUAL
		/// Called right before the shader is linked
		/// Use this to bind your attribute locations
		virtual void BindAttributes() = 0;

		/// VIRTUAL
		/// Called right after the shader is linked
		/// Use this to get your uniform locations
		virtual void RegisterUniforms() = 0;

		/// Virtual
		/// Used for setting up global shader variables like matrices or other usefull data
		/// Normally using 'loadX' functions (or wrappers for those functions)
		virtual void OnBind() = 0;


		/// SHADER COMPILATION AND LINKAGE

		/// \param errorLog If any error is encountered during shader compilation
		/// 	A log will be generated inside errorLog
		///
		int compile(GLuint &hdl, GLenum type, const char* data, std::string& errorLog);
		int retrieveSourceFromFile(const std::string& file, std::string& out) const;

		int linkShaders(std::vector<GLuint>& shaders);

		//
		template <typename ...Args>
		int CompileFromFile(std::vector<GLuint>& shaders){ return BE_NO_ERROR; }
		template <typename ...Args>
		int CompileFromMemory(std::vector<GLuint>& shaders){ return BE_NO_ERROR; }

		int BuildFinalProgram(std::vector<GLuint>& shaders);

		template <typename ...Args>
		int CompileFromFile(std::vector<GLuint>& shaders, GLint type, const std::string& file, Args... args)
		{
			LOG(EngineLog, BE_LOG_VERBOSE) << "Compiling shader (type: " << type << ") file " << file;
			std::string source;
			if (retrieveSourceFromFile(file, source) != BE_NO_ERROR){
				LOG(EngineLog, BE_LOG_ERROR) << "Failed to read shader file: " << file;
				return FAILED_TO_READ;
			}

			if (CompileFromMemory(shaders, type, source.data()) != BE_NO_ERROR){
				return FAILED_TO_COMPILE;
			}

			return CompileFromFile(shaders, args...);
		}

		template <typename ...Args>
		int CompileFromMemory(std::vector<GLuint>& shaders, GLint type, const char* source, Args... args)
		{
			if (CompileFromMemory(shaders, type, source) != BE_NO_ERROR){
				return FAILED_TO_COMPILE;
			}

			return CompileFromMemory(shaders, args...);
		}

		int CompileFromMemory(std::vector<GLuint>& shaders, GLint type, const char* source);

		/// Destroy shaders
		void FreeShaders();

};

} // namespace BitEngine
