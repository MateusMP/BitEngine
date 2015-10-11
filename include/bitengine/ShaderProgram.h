#pragma once

#include <string>
#include <vector>

#include "TypeDefinition.h"

#include "Graphics.h"
#include "EngineLoggers.h"
#include "ErrorCodes.h"


#ifdef _DEBUG
#define LOAD_UNIFORM(var, name)																						\
				var = getUniformLocation(name);																		\
				if (var < 0)																						\
					LOGTO(Warning) << "Shader: Failed to load "#var" [\""name"\"] uniform." << BitEngine::endlog;	\
				else																								\
					LOGTO(Verbose) << "Uniform "#var" loaded with id: "<< var << "." << BitEngine::endlog
#else
#define LOAD_UNIFORM(var, name)	\
					var = getUniformLocation(name)
#endif

#define ARRAY_OFFSET(x) ((void*)(x))

#define VERTEX_MATRIX3_ATTIBUTE_SIZE 3
#define VERTEX_MATRIX4_ATTIBUTE_SIZE 4

namespace BitEngine{
	
class ShaderProgram
{
    public:
        ShaderProgram();
        virtual ~ShaderProgram();

		virtual int Init() = 0;

		/**
		 * Binds the shader
		 * Calls OnBind();
		 */
        void Bind();

		/**
		 * Unbinds the shader
		 */
        void Unbind();

		/// VIRTUAL
		// \param outVBO vector to store all VBO's id
		// Returns a VAO with all ShaderConfigurations Set
		// GLuint CreateVAO(GLuint* outVBO) = 0;

        /// VIRTUAL
        ///
        ///
        virtual void BindAttributes() = 0;

        /// VIRTUAL
        ///
        ///
        virtual void RegisterUniforms() = 0;

		/// Virtual
		virtual void OnBind() = 0;

		/**
		* Build and link a set of sources from file to create a final Shader Program
		* @return Returns NO_ERROR in case of success
		*/
		template <typename ...Args>
		int BuildProgramFromFile(GLint type, const std::string& file, Args... args){
			std::vector<GLuint> shaders;

			if (CompileFromFile(shaders, type, file, args...) == NO_ERROR){
				return BuildFinalProgram(shaders);
			}

			return FAILED_TO_COMPILE;
		}

		/**
		 * Build and link a set of sources from memory to create a final Shader Program
		 * @return Returns NO_ERROR in case of success
		 */
		template <typename ...Args>
		int BuildProgramFromMemory(GLint type, const char* source, Args... args){
			std::vector<GLuint> shaders;

			if (CompileFromMemory(shaders, type, source, args...) == NO_ERROR){
				return BuildFinalProgram(shaders);
			}

			return FAILED_TO_COMPILE;
		}

	protected:
        GLuint m_programID;

		// Functions

		/// Bind attribute ID to specified variable name on shader
		void BindAttribute(int attrid, const std::string& name);

		/// Get uniform location on shader
		int32 getUniformLocation(const std::string& name) const;

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

		/// \param textureID GL flag to indicate texture unit (GL_TEXTURE0 ... )
		void connectTexture(int location, int unitID);

		/**
		 * \param errorLog If any error is encountered during shader compilation
		 *		A log will be generated inside errorLog
		 */
		int compile(GLuint &hdl, GLenum type, const char* data, std::string& errorLog);
		int retrieveSourceFromFile(const std::string& file, std::string& out) const;

		void linkShaders();
		int linkShaders(std::vector<GLuint>& shaders);

	private:
		template <typename ...Args>
		int CompileFromFile(std::vector<GLuint>& shaders){ return NO_ERROR; }
		template <typename ...Args>
		int CompileFromMemory(std::vector<GLuint>& shaders){ return NO_ERROR; }

		int BuildFinalProgram(std::vector<GLuint>& shaders);

		template <typename ...Args>
		int CompileFromFile(std::vector<GLuint>& shaders, GLint type, const std::string& file, Args... args)
		{
			LOGTO(Verbose) << "Compiling shader (type: " << type << ") file " << file << endlog;
			std::string source;
			if (retrieveSourceFromFile(file, source) != NO_ERROR){
				LOGTO(Error) << "Failed to read shader file: " << file << endlog;
				return FAILED_TO_READ;
			}

			if (CompileFromMemory(shaders, type, source.data()) != NO_ERROR){
				return FAILED_TO_COMPILE;
			}

			return CompileFromFile(shaders, args...);
		}

		template <typename ...Args>
		int CompileFromMemory(std::vector<GLuint>& shaders, GLint type, const char* source, Args... args)
		{
			if (CompileFromMemory(shaders, type, source) != NO_ERROR){
				return FAILED_TO_COMPILE;
			}

			return CompileFromMemory(shaders, args...);
		}

		int CompileFromMemory(std::vector<GLuint>& shaders, GLint type, const char* source);

		/// Destroy shaders
		void FreeShaders();

};

}