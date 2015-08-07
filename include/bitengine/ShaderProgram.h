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

		/// Bind attribute ID to specified variable name on shader
        void BindAttribute(int attrid, const std::string& name);

		/// Get uniform location on shader
		int32 getUniformLocation(const std::string& name) const;

        /// Load a single INT uniform
        void loadInt(int location, int value);
        void loadFloat(int location, float value);

        void loadVector3f(int location, int n, float* vector);
        void loadVector4f(int location, int n, float* vector);

        void loadBoolean(int location, bool b);

        void loadMatrix(int location, float* matrix);

		/// \param textureID GL flag to indicate texture unit (GL_TEXTURE0 ... )
		void connectTexture(int location, int unitID);

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