#pragma once

#include <string>
#include <vector>

#include "TypeDefinition.h"

#include "Graphics.h"

namespace BitEngine{


template<typename... Ts>
class IInstancedRenderer{
	virtual void begin() = 0;
	virtual void end() = 0;
	virtual void Render() = 0;

	virtual void AddInstance(Ts&&... t) = 0;

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

};


class ShaderProgram
{
    public:
        ShaderProgram();
        virtual ~ShaderProgram();

        int compileShaders(const std::string& vertexFile, const std::string& fragmentFile);

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

        std::vector<std::string> m_shaderFiles;

		// Functions

        GLuint compileSource(GLuint &hdl, GLenum type, const std::string& file);

		void linkShaders();

};



}