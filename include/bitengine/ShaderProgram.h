#pragma once

#include <string>
#include <vector>

#include "TypeDefinition.h"

#include "Graphics.h"
#include "EngineLoggers.h"
#include "ErrorCodes.h"


#ifdef _DEBUG
#define LOAD_UNIFORM(var, name)																					\
				var = getUniformLocation(name);																	\
				if (var < 0)																					\
					LOGTO(Warning) << "Shader: Failed to load "#var" [\""name"\"] uniform." << endlog;			\
				else																							\
					LOGTO(Verbose) << "Uniform "#var" loaded with id: "<< var << "." << endlog
#else
#define LOAD_UNIFORM(var, name)	\
					var = getUniformLocation(name)
#endif

namespace BitEngine{
	
class ShaderProgram
{
    public:

        ShaderProgram();
        virtual ~ShaderProgram();

		int CompileShadersFiles(const std::string& vertexFile, const std::string& fragmentFile);
		int CompileShadersSources(const char* vertexSource, const char* fragmentSource);

        void Bind();
        void Unbind();

		/// VIRTUAL
		// \param outVBO vector to store all VBO's id
		// Returns a VAO with all ShaderConfigurations Set
		virtual GLuint CreateVAO(GLuint* outVBO) = 0;

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

	protected:
        GLuint m_programID;
        GLuint m_vertexID;
        GLuint m_fragmentID;

		// Functions

		/// \param errorLog, pointer to char* where to store compilation errors
		///			Should be freed with delete[] after use.
		int compile(GLuint &hdl, GLenum type, const char* data, char** errorLog);
		int retrieveSourceFromFile(const std::string& file, std::string& out) const;

		void linkShaders();

};

template<typename... Ts>
class IInstancedRenderer{
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void Render() = 0;

	virtual void AddInstance(Ts&&... t) = 0;

	virtual ShaderProgram* getShader() = 0;
};

class IBatchRenderer{
public:
	enum class BATCH_MODE{
		STATIC, STATIC_DEFINED,
		DYNAMIC,
		STREAM,
	};

	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void Render() = 0;

	virtual ShaderProgram* getShader() = 0;
};


}