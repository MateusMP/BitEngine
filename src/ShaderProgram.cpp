
#include <fstream>

#include "ShaderProgram.h"
#include "Logger.h"

#include "glSystem.h"

namespace BitEngine{


ShaderProgram::ShaderProgram()
    : m_programID(0), m_vertexID(0), m_fragmentID(0)
{
    m_shaderFiles.resize(2);
}

ShaderProgram::~ShaderProgram()
{
	glDeleteProgram(m_programID);
}

void ShaderProgram::Bind()
{
    glUseProgram(m_programID);
}

void ShaderProgram::Unbind()
{
    glUseProgram(0);
}

int ShaderProgram::compileShaders(const std::string& vertexFile, const std::string& fragmentFile)
{
    m_shaderFiles[0] = vertexFile;
    m_shaderFiles[1] = fragmentFile;

    int error = compileSource(m_vertexID, GL_VERTEX_SHADER, vertexFile);
    if (error < 0){
        Logger::LogErrorToConsole("ShaderProgram: Failed to create VertexShader [error: %d - %s]\n", m_vertexID, vertexFile.c_str());
        Logger::ABORT(2);
    }

    error = compileSource(m_fragmentID , GL_FRAGMENT_SHADER, fragmentFile);
    if (error < 0){
        Logger::LogErrorToConsole("ShaderProgram: Failed to create FragmentShader [error: %d - %s]\n", m_fragmentID, fragmentFile.c_str());
        Logger::ABORT(2);
    }

    // Vertex and fragment shaders are successfully compiled.
    // Now time to link them together into a program.
    // Get a program object.
    m_programID = glCreateProgram();

    BindAttributes();

    linkShaders();

    RegisterUniforms();

    return 1;
}

void ShaderProgram::BindAttribute(int attrib, const std::string& name)
{
    glBindAttribLocation(m_programID, attrib, name.c_str() );
}

int32 ShaderProgram::getUniformLocation(const std::string& name) const
{
	return glGetUniformLocation(m_programID, name.c_str());
}

void ShaderProgram::linkShaders()
{
    //Attach our shaders to our program
    glAttachShader(m_programID, m_vertexID);
    glAttachShader(m_programID, m_fragmentID);

    //Link our program
    glLinkProgram(m_programID);

    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    glGetProgramiv(m_programID, GL_LINK_STATUS, (int *)&isLinked);
    if(isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        glGetProgramInfoLog(m_programID, maxLength, &maxLength, &infoLog[0]);

        // We don't need the program anymore.
        glDeleteProgram(m_programID);

        // Don't leak shaders
        glDeleteShader(m_vertexID);
        glDeleteShader(m_fragmentID);

        // Use the infoLog as you see fit.
        Logger::LogErrorToConsole("Linking error: %s", &infoLog[0]);

        // In this simple program, we'll just leave
        return;
    }

    // Always detach shaders after a successful link.
    glDetachShader(m_programID, m_vertexID);
    glDetachShader(m_programID, m_fragmentID);

    // Free shaders ids
    glDeleteShader(m_vertexID);
    glDeleteShader(m_fragmentID);
}

GLuint ShaderProgram::compileSource(GLuint &hdl, GLenum type, const std::string& file)
{
    GLuint shdhdl = glCreateShader(type);
    if (shdhdl == 0){
        return -1;
    }

    std::ifstream fdata(file);
    if (fdata.fail()){
        return -2;
    }

    std::string filedata;
    std::string linedata;

    while(std::getline(fdata, linedata))
    {
        filedata += linedata + "\n";
    }

    fdata.close();

    const char* data = filedata.c_str();
    glShaderSource(shdhdl, 1, &data, nullptr);
    glCompileShader(shdhdl);

    GLint status = 0;
    glGetShaderiv(shdhdl, GL_COMPILE_STATUS, &status);

    if (status == GL_FALSE)
    {
        GLint maxlogsize;
        glGetShaderiv(shdhdl, GL_INFO_LOG_LENGTH, &maxlogsize);

        std::vector<char> log(maxlogsize);
        glGetShaderInfoLog(shdhdl, maxlogsize, &maxlogsize, &log[0]);

        // Free shader handle
        glDeleteShader(shdhdl);

        Logger::LogErrorToConsole("Shader %s compile Error!\n%s\n====\n", file.c_str(), &(log[0]) );

        return -3;
    }

    hdl = shdhdl;
    return 0;
}




void ShaderProgram::loadInt(int location, int value){
    glUniform1i(location, value);
}

void ShaderProgram::loadFloat(int location, float value){
    glUniform1f(location, value);
}

void ShaderProgram::loadVector3f(int location, int n, float* vector){
    glUniform3fv(location, n, vector);
}

void ShaderProgram::loadVector4f(int location, int n, float* vector){
    glUniform4fv(location, n, vector);
}

void ShaderProgram::loadBoolean(int location, bool b){
    float toload = 0.0f;
    if (b)
        toload = 1.0f;
    glUniform1f(location, toload);
}

void ShaderProgram::loadMatrix(int location, float* matrix){
    glUniformMatrix4fv(location, 1, false, matrix);
}

void ShaderProgram::connectTexture(int location, int unitID){
	glUniform1i(location, unitID);
}


}
