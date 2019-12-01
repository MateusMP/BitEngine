#pragma once

#include "GL2Batch.h"

namespace BitEngine
{

enum ShaderSourceTypes : u8 {
    SourceNone = 0,
    SourceVertex = 1,
    SourceFragment = 2,
    SourceGeometry = 4,
};
inline ShaderSourceTypes operator|(ShaderSourceTypes a, ShaderSourceTypes b) {
    return static_cast<ShaderSourceTypes>(static_cast<int>(a) | static_cast<int>(b));
}
inline bool operator&(ShaderSourceTypes a, ShaderSourceTypes b) {
    return (static_cast<int>(a) & static_cast<int>(b)) != 0;
}

class GL2Shader : public Shader
{
    friend class GL2ShaderManager;
public:
    GL2Shader() : Shader(nullptr), m_programID(0) {}
    GL2Shader(ResourceMeta* meta);
    virtual ~GL2Shader();

    int init();

    bool isReady() override { return m_programID != 0; }

    /// Binds the shader
    /// Calls OnBind()
    void bind() override;

    /// Unbinds the shader
    void Unbind() override;

    IGraphicBatch* createBatch() override;

    // Load data into uniform referenced with ref
    void loadConfig(const UniformDefinition* ref, const void* data);

    void includeSource(GLint type, std::vector<char>& data);

    ShaderDataDefinition& getDefinition() override { return m_shaderDefinition; }

    void setExpectedShaderSourcesCount(int amount) { expectedSourcesCount = amount; }

protected:
    struct ShaderSource {
        ShaderSource(GLint t, std::vector<char>& d)
            : type(t), shader(0)
        {
            data.swap(d);
        }

        GLint type;
        GLuint shader; // compiled
        std::vector<char> data;
    };

    void introspect();
    // ATTRIBUTE/UNIFORM FUNCTIONS

    /// Bind attribute ID to specified variable name on shader
    /// Usually called inside BindAttributes implementation
    void bindAttribute(int attrid, const std::string& name);

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
    void compileSources();
    /// VIRTUAL
    // \param outVBO vector to store all VBO's id
    // Returns a VAO with all ShaderConfigurations Set
    // GLuint CreateVAO(GLuint* outVBO) = 0;

    /// SHADER COMPILATION AND LINKAGE

    /// \param errorLog If any error is encountered during shader compilation
    /// 	A log will be generated inside errorLog
    ///
    int compile(GLenum type, const std::vector<char>& data, GLuint &hdl, std::string& errorLog);

    int linkShaders(std::vector<GLuint>& shaders);

    int buildFinalProgram(std::vector<GLuint>& shaders);

    /// Destroy shaders
    void releaseShader();

    void registerAttributes() {}
    void registerUniforms() {}

    void genUniformContainer(UniformHolder& unifContainer);
    void genVBOAttributes(VAOContainer& vaoContainer);
    VAOContainer genVAOArrays(const VAOContainer& base);
    UniformDefinition* findUniformConfigByName(const std::string& str);
    VBOAttrib* findAttributeConfigByName(const std::string& str);

    // members
    GLuint m_programID; //!< program unique id
    u16 expectedSourcesCount;

    std::vector<ShaderSource> sources;

    ShaderDataDefinition m_shaderDefinition;
    std::vector<VBOAttrib> m_attributes;
    std::vector<UniformDefinition> m_uniforms;

    VAOContainer baseVaoContainer;
    UniformHolder uniformHolder;

    std::vector<GL2Batch*> batches;

    std::function<void()> reload;
};

}
