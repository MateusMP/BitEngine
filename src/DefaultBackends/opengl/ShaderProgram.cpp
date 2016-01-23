
#include <fstream>

#include "DefaultBackends/opengl/ShaderProgram.h"
#include "DefaultBackends/opengl/glSystem.h"

#include "Core/Logger.h"

namespace BitEngine{

ShaderProgram::ShaderProgram()
	: m_programID(0)
{
}

ShaderProgram::~ShaderProgram()
{
	FreeShaders();
}

void ShaderProgram::Bind()
{
    glUseProgram(m_programID);
	OnBind();
}

void ShaderProgram::Unbind()
{
    glUseProgram(0);
}

void ShaderProgram::FreeShaders()
{
	if (m_programID != 0)
	{
		Unbind();
		glDeleteProgram(m_programID);
		m_programID = 0;
	}
}

void ShaderProgram::BindAttribute(int attrib, const std::string& name)
{
    glBindAttribLocation(m_programID, attrib, name.c_str() );
}

int32 ShaderProgram::getUniformLocation(const std::string& name) const
{
	return glGetUniformLocation(m_programID, name.c_str());
}

int ShaderProgram::BuildFinalProgram(std::vector<GLuint>& shaders)
{
	FreeShaders();

	m_programID = glCreateProgram();

	BindAttributes();

	if (linkShaders(shaders) == BE_NO_ERROR){
		RegisterUniforms();
		return BE_NO_ERROR;
	}
	else {
		return FAILED_TO_LINK;
	}
}

int ShaderProgram::CompileFromMemory(std::vector<GLuint>& shaders, GLint type, const char* source)
{
	GLuint shaderHDL;
	std::string errorlog;

	int error = compile(shaderHDL, type, source, errorlog);
	if (error != BE_NO_ERROR){
		LOG(EngineLog, BE_LOG_ERROR) << "Shader: Shader <m> compile Error!\n " << errorlog;
		return FAILED_TO_COMPILE;
	}
	else {
		shaders.push_back(shaderHDL);
	}

	return BE_NO_ERROR;
}

int ShaderProgram::linkShaders(std::vector<GLuint>& shaders)
{
	//Attach our shaders to our program
	for (size_t i = 0; i < shaders.size(); ++i){
		glAttachShader(m_programID, shaders[i]);
	}

	//Link our program
	glLinkProgram(m_programID);

	// Note the different functions here: glGetProgram* instead of glGetShader*.
	GLint isLinked = 0;
	glGetProgramiv(m_programID, GL_LINK_STATUS, (int *)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

		// The maxLength includes the NULL character
		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(m_programID, maxLength, &maxLength, &infoLog[0]);

		// We don't need the program anymore.
		glDeleteProgram(m_programID);

		// Don't leak shaders
		for (size_t i = 0; i < shaders.size(); ++i){
			glDeleteShader(shaders[i]);
		}

		// Use the infoLog as you see fit.
		LOG(EngineLog, BE_LOG_ERROR) << "Shader: Linking error: " << &infoLog[0];

		// In this simple program, we'll just leave
		return FAILED_TO_LINK;
	}

	// Always detach shaders after a successful link.
	for (size_t i = 0; i < shaders.size(); ++i){
		glDetachShader(m_programID, shaders[i]);
	}

	// Free shaders ids
	for (size_t i = 0; i < shaders.size(); ++i){
		glDeleteShader(shaders[i]);
	}

	return BE_NO_ERROR;
}

int ShaderProgram::retrieveSourceFromFile(const std::string& file, std::string& out) const
{
	out.clear();

	std::ifstream fdata(file);
	if (fdata.fail()){
		return FAILED_TO_READ;
	}

	std::string linedata;

	while (std::getline(fdata, linedata))
	{
		out += linedata + "\n";
	}

	fdata.close();

	return BE_NO_ERROR;
}

int ShaderProgram::compile(GLuint &hdl, GLenum type, const char* data, std::string& errorLog)
{
    GLuint shdhdl = glCreateShader(type);
    if (shdhdl == 0){
		return FAILED_TO_CREATE_SHADER;
    }

	glShaderSource(shdhdl, 1, &data, nullptr);
	glCompileShader(shdhdl);

	GLint status = 0;
	glGetShaderiv(shdhdl, GL_COMPILE_STATUS, &status);

	if (status == GL_FALSE){

		GLint maxlogsize;
		glGetShaderiv(shdhdl, GL_INFO_LOG_LENGTH, &maxlogsize);

		errorLog.clear();
		errorLog.resize(maxlogsize + 1);

		glGetShaderInfoLog(shdhdl, maxlogsize, &maxlogsize, (GLchar*)errorLog.data());

		// Free shader handle
		glDeleteShader(shdhdl);
		
		return FAILED_TO_COMPILE;
	}

    hdl = shdhdl;
    return BE_NO_ERROR;
}




void ShaderProgram::loadInt(int location, int value){
    glUniform1i(location, value);
}

void ShaderProgram::loadFloat(int location, float value){
    glUniform1f(location, value);
}

void ShaderProgram::loadVector3f(int location, int n, const float* vector){
    glUniform3fv(location, n, vector);
}

void ShaderProgram::loadVector4f(int location, int n, const float* vector){
    glUniform4fv(location, n, vector);
}

void ShaderProgram::loadBoolean(int location, bool b){
    float toload = 0.0f;
    if (b)
        toload = 1.0f;
    glUniform1f(location, toload);
}

void ShaderProgram::loadMatrix4f(int location, const float* matrix){
    glUniformMatrix4fv(location, 1, false, matrix);
}

void ShaderProgram::connectTexture(int location, int unitID){
	glUniform1i(location, unitID);
}


}
