#include "Platform/opengl/GL2/GL2Shader.h"
#include "Platform/opengl/GL2/GL2Batch.h"

#include "Platform/opengl/GL2/GL2TextureManager.h"
#include "Platform/opengl/GL2/GL2Impl.h"

#include "BitEngine/Common/ErrorCodes.h"
#include "BitEngine/Core/Assert.h"

namespace BitEngine {
u32 sizeofDataType(DataType dt)
{
    BE_ASSERT((dt.value != DataType::INVALID_DATA_TYPE) && "Unexpected data type.");

    switch (dt.value) {
    case DataType::FLOAT:
        return sizeof(float);
    case DataType::LONG:
        return sizeof(long);
    case DataType::MAT2:
        return sizeof(float) * 2 * 2;
    case DataType::MAT3:
        return sizeof(float) * 3 * 3;
    case DataType::MAT4:
        return sizeof(float) * 4 * 4;
    case DataType::VEC2:
        return sizeof(float) * 2;
    case DataType::VEC3:
        return sizeof(float) * 3;
    case DataType::VEC4:
        return sizeof(float) * 4;
    case DataType::TEXTURE_1D:
        return sizeof(GL2Texture*);
    case DataType::TEXTURE_2D:
        return sizeof(GL2Texture*);
    case DataType::TEXTURE_3D:
        return sizeof(GL2Texture*);
    case DataType::TEXTURE_CUBE:
        return sizeof(GL2Texture*);
    case DataType::TOTAL_DATA_TYPES:
    case DataType::INVALID_DATA_TYPE:
        return 0;
    }
    return 0;
}

// ###############################################################
// GL2Shader
UniformDefinition* GL2Shader::findUniformConfigByName(const std::string& str)
{
    const std::string u_str = "u_" + str;
    for (UniformDefinition& gc : m_uniforms) {
        if (gc.name == u_str) {
            return &gc;
        }
    }

    return nullptr;
}

VBOAttrib* GL2Shader::findAttributeConfigByName(const std::string& str)
{
    const std::string a_str = "a_" + str;
    for (VBOAttrib& ac : m_attributes) {
        if (ac.name == a_str) {
            return &ac;
        }
    }

    return nullptr;
}

GL2Shader::GL2Shader(ResourceMeta* meta)
    : Shader(meta)
    , m_programID(0)
    , npieces(0)
{
}

GL2Shader::~GL2Shader()
{
    releaseShader();
}

IGraphicBatch* GL2Shader::createBatch()
{
    if (!isReady()) {
        return nullptr;
    }
    // VAO contains all containers from ShaderDataDefinition
    // Each VBO is one container.
    // Each VBO is configured based on the container data.
    // EX:
    //  def.addContainer(DataUseMode::Vertex, 0)
    //  	.addVertexData("position", DataType::VEC3, 1)
    //  	.addVertexData("textureUV", DataType::VEC4, 1);
    //
    //  def.addContainer(DataUseMode::Vertex, 1)
    //  	.addVertexData("modelMatrix", DataType::MAT4, 1);
    // GIVES:
    // VAO
    //  |-> VBO 0 { vec3, vec4 }			 at attrId 0 .. 1, divisor = 0
    //  |-> VBO 1 { vec4, vec4, vec4, vec4 } at attrId 2 .. 5, divisor = 1

    GL2Batch* batch = new GL2Batch(genVAOArrays(baseVaoContainer), uniformHolder);

    return batch;
}

void GL2Shader::setupBatch(Lazy<GL2Batch>& batch)
{

    // VAO contains all containers from ShaderDataDefinition
    // Each VBO is one container.
    // Each VBO is configured based on the container data.
    // EX:
    //  def.addContainer(DataUseMode::Vertex, 0)
    //  	.addVertexData("position", DataType::VEC3, 1)
    //  	.addVertexData("textureUV", DataType::VEC4, 1);
    //
    //  def.addContainer(DataUseMode::Vertex, 1)
    //  	.addVertexData("modelMatrix", DataType::MAT4, 1);
    // GIVES:
    // VAO
    //  |-> VBO 0 { vec3, vec4 }			 at attrId 0 .. 1, divisor = 0
    //  |-> VBO 1 { vec4, vec4, vec4, vec4 } at attrId 2 .. 5, divisor = 1

    batch.destroy();
    if (isReady()) {
        new (&batch) Lazy<GL2Batch>(genVAOArrays(baseVaoContainer), uniformHolder);
        batch.initialized();
    }
}

/// Init the shader
/// Normally calls BuildProgramFromFile/Memory
int GL2Shader::init()
{
    BE_PROFILE_FUNCTION();
    LOG(EngineLog, BE_LOG_INFO) << "Building shader...";

    int error = buildFinalProgram();
    if (error == BE_NO_ERROR) {
        LOG(EngineLog, BE_LOG_INFO) << "Shader ready!";
    }
    else {
        LOG(EngineLog, BE_LOG_INFO) << "Shader initialization failed " << error;
    }

    return error;
}

/// Binds the shader
/// Calls OnBind()
void GL2Shader::bind()
{
    GL2::bindShaderProgram(m_programID);
}

/// Unbinds the shader
void GL2Shader::Unbind()
{
    GL2::bindShaderProgram(0);
}

void GL2Shader::releaseShader()
{
    if (m_programID != 0) {
        Unbind();
        GL_CHECK(glDeleteProgram(m_programID));
        m_programID = 0;
    }
    npieces = 0;
}

void GL2Shader::introspect()
{
    BE_PROFILE_FUNCTION();
    GLint nameReadSize;
    GLchar nameBuffer[128] = {};

    // Introspect ATTRIBUTES
    GLint nAttrs;
    GL_CHECK(glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTES, &nAttrs));
    GL_CHECK(glGetProgramiv(m_programID, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &nameReadSize));
    BE_ASSERT(nameReadSize < sizeof(nameBuffer));

    for (int i = 0; i < nAttrs; ++i) {
        VBOAttrib attr;

        // TODO: Handle big attributes like matrices
        glGetActiveAttrib(m_programID, i, sizeof(nameBuffer), &nameReadSize, &attr.size, &attr.type, nameBuffer);
        std::string_view tmpName(nameBuffer, nameReadSize);
        attr.id = glGetAttribLocation(m_programID, tmpName.data());
        attr.name = tmpName;
        attr.dataSize = GL2::fromGLTypeToGLDataTypeSize(attr.type);
        attr.dataType = GL2::fromGLTypeToGLDataType(attr.type);

        // Find divisor value
        attr.divisor = 0;
        std::string::size_type instAt = tmpName.find("inst_");
        if (instAt != std::string::npos) {
            if ((instAt + 4) < tmpName.size()) {
                char div = tmpName[instAt + 5];
                attr.divisor = div - '0';
            }
        }

        LOG(EngineLog, BE_LOG_INFO) << "Attr" << i << " at " << attr.id
                                    << " '" << nameBuffer << "' is of type: " << attr.type << " size: " << attr.size;

        m_attributes.emplace_back(attr);
    }
    // Load attributes
    genVBOAttributes(baseVaoContainer);

    // Introspect UNIFORMS
    GLint nUnif;
    GL_CHECK(glGetProgramiv(m_programID, GL_ACTIVE_UNIFORMS, &nUnif));
    GL_CHECK(glGetProgramiv(m_programID, GL_ACTIVE_UNIFORM_MAX_LENGTH, &nameReadSize));
    BE_ASSERT(nameReadSize < sizeof(nameBuffer));

    for (int i = 0; i < nUnif; ++i) {
        UniformDefinition unif;

        GL_CHECK(glGetActiveUniform(m_programID, i, sizeof(nameBuffer), &nameReadSize, &unif.size, &unif.type, nameBuffer));
        std::string_view tmpName(nameBuffer, nameReadSize);

        unif.location = glGetUniformLocation(m_programID, tmpName.data());
        GL_CHECK(;);
        unif.name = tmpName;
        LOG(EngineLog, BE_LOG_INFO) << "Uniform" << i << " at " << unif.location
                                    << " '" << nameBuffer << "' is of type: " << unif.type << " size: " << unif.size;
        m_uniforms.emplace_back(unif);
    }
    genUniformContainer(uniformHolder);
}

void GL2Shader::genVBOAttributes(VAOContainer& vaoContainer)
{
    BE_PROFILE_FUNCTION();
    const std::vector<ShaderDataDefinition::DefinitionContainer>& containers = m_shaderDefinition.getContainers(DataUseMode::Vertex);

    for (const ShaderDataDefinition::DefinitionContainer& dc : containers) {
        int strideSize = 0;
        for (const ShaderDataDefinition::DefinitionData& dd : dc.definitionData) {
            strideSize += sizeofDataType(dd.type) * dd.size;
        }

        VBOContainer vboc;

        u32 offsetAccum = 0;
        for (const ShaderDataDefinition::DefinitionData& dd : dc.definitionData) {
            VBOAttrib* ac = findAttributeConfigByName(dd.name);
            if (ac != nullptr) {
                BE_ASSERT(GL2::toGLType(dd.type) == ac->type);
                ac->normalized = 0; // TODO: get this from dd
                ac->stride = strideSize;
                ac->offset = offsetAccum;

                offsetAccum += sizeofDataType(dd.type) * dd.size;

                vboc.attrs.emplace_back(ac);
            }
            else {
                LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Couldn't find shader related attribute " << dd.name;
            }
        }
        vboc.divisor = dc.instanced;
        vboc.stride = strideSize;
        vboc.ref = ShaderDataReference(dc.mode, dc.container, 0);

        vaoContainer.vbos.emplace_back(vboc);
    }
}

VAOContainer GL2Shader::genVAOArrays(const VAOContainer& base)
{
    BE_PROFILE_FUNCTION();
    VAOContainer container;
    GL2::genVao(1, &container.vao);
    GL2::bindVao(container.vao);

    for (VBOContainer vboc : base.vbos) {
        GL2::genVbo(1, &vboc.vbo);
        if (vboc.vbo == 0) {
            LOG(EngineLog, BE_LOG_ERROR) << "VertexBuffer: Could not create VBO.";
            GL2::deleteVaos(1, &container.vao);
            container.vao = 0;
            return container;
        }

        for (const VBOAttrib* c : vboc.attrs) {
            GL2::setupVbo(c->id, vboc.vbo, c->dataSize, c->dataType, c->normalized, c->stride, c->offset, vboc.divisor);
        }

        container.vbos.emplace_back(vboc);
    }

    GL2::unbindVao();
    return container;
}

void GL2Shader::genUniformContainer(UniformHolder& unifContainer)
{
    BE_PROFILE_FUNCTION();
    const std::vector<ShaderDataDefinition::DefinitionContainer>& containers = m_shaderDefinition.getContainers(DataUseMode::Uniform);

    for (const ShaderDataDefinition::DefinitionContainer& def : containers) {
        unifContainer.containers.emplace_back(ShaderDataReference(DataUseMode::Uniform, def.container, 0), def.instanced);
        UniformContainer& container = unifContainer.containers.back();
        u32 fullSize = 0;
        for (const ShaderDataDefinition::DefinitionData& dd : def.definitionData) {
            UniformDefinition* ud = findUniformConfigByName(dd.name);
            if (ud != nullptr) {
                u32 partialSize = sizeofDataType(dd.type) * dd.size;
                ud->instanced = def.instanced;
                ud->byteSize = partialSize;
                ud->dataOffset = fullSize;
                ud->extraInfo = 0;
                fullSize += partialSize;

                container.defs.emplace_back(ud);
            }
            container.stride = fullSize;
        }
    }
}

void GL2Shader::bindAttribute(int attrib, const std::string& name)
{
    GL_CHECK(glBindAttribLocation(m_programID, attrib, name.c_str()));
}

s32 GL2Shader::getUniformLocation(const std::string& name) const
{
    return glGetUniformLocation(m_programID, name.c_str());
}

int GL2Shader::buildFinalProgram()
{
    BE_PROFILE_FUNCTION();
    m_programID = glCreateProgram();

    registerAttributes();

    GL_CHECK(;);
    if (linkShaders() == BE_NO_ERROR) {
        registerUniforms();
        GL_CHECK(;);
        return BE_NO_ERROR;
    }
    else {
        return FAILED_TO_LINK;
    }
}

int GL2Shader::linkShaders()
{
    //Attach our shaders to our program
    for (size_t i = 0; i < npieces; ++i) {
        GL_CHECK(glAttachShader(m_programID, pieces[i]));
    }

    //Link our program
    GL_CHECK(glLinkProgram(m_programID));

    // Note the different functions here: glGetProgram* instead of glGetShader*.
    GLint isLinked = 0;
    GL_CHECK(glGetProgramiv(m_programID, GL_LINK_STATUS, (int*)&isLinked));
    if (isLinked == GL_FALSE) {
        GLint maxLength = 0;
        GL_CHECK(glGetProgramiv(m_programID, GL_INFO_LOG_LENGTH, &maxLength));

        // The maxLength includes the NULL character
        std::vector<GLchar> infoLog(maxLength);
        GL_CHECK(glGetProgramInfoLog(m_programID, maxLength, &maxLength, &infoLog[0]));

        // We don't need the program anymore.
        GL_CHECK(glDeleteProgram(m_programID));

        // Don't leak shaders
        for (size_t i = 0; i < npieces; ++i) {
            GL_CHECK(glDetachShader(m_programID, pieces[i]));
            GL_CHECK(glDeleteShader(pieces[i]));
        }

        // Use the infoLog as you see fit.
        LOG(EngineLog, BE_LOG_ERROR) << "Shader: Linking error: " << &infoLog[0];

        // In this simple program, we'll just leave
        return FAILED_TO_LINK;
    }

    // Always detach shaders after a successful link.
    for (size_t i = 0; i < npieces; ++i) {
        GL_CHECK(glDetachShader(m_programID, pieces[i]));
    }

    // Free shaders ids
    for (size_t i = 0; i < npieces; ++i) {
        GL_CHECK(glDeleteShader(pieces[i]));
    }

    introspect();

    return BE_NO_ERROR;
}

// \param hdl Where to save the gl id for the shader
int GL2Shader::compile(GLenum type, const void* data, GLint size, GLuint& hdl, std::string& errorLog)
{
    BE_PROFILE_FUNCTION();
    GLuint shdhdl = glCreateShader(type);
    if (shdhdl == 0) {
        return FAILED_TO_CREATE_SHADER;
    }

    const char* codeData = (const char*)data;
    GL_CHECK(glShaderSource(shdhdl, 1, &codeData, &size));
    GL_CHECK(glCompileShader(shdhdl));

    GLint status = 0;
    GL_CHECK(glGetShaderiv(shdhdl, GL_COMPILE_STATUS, &status));

    if (status == GL_FALSE) {

        GLint maxlogsize;
        GL_CHECK(glGetShaderiv(shdhdl, GL_INFO_LOG_LENGTH, &maxlogsize));

        errorLog.clear();
        errorLog.resize(maxlogsize + 1);

        GL_CHECK(glGetShaderInfoLog(shdhdl, maxlogsize, &maxlogsize, (GLchar*)errorLog.data()));

        // Free shader handle
        GL_CHECK(glDeleteShader(shdhdl));

        return FAILED_TO_COMPILE;
    }

    hdl = shdhdl;
    return BE_NO_ERROR;
}

void GL2Shader::loadInt(int location, int value)
{
    GL_CHECK(glUniform1i(location, value));
}

void GL2Shader::loadFloat(int location, float value)
{
    GL_CHECK(glUniform1f(location, value));
}

void GL2Shader::loadVector3f(int location, int n, const float* vector)
{
    GL_CHECK(glUniform3fv(location, n, vector));
}

void GL2Shader::loadVector4f(int location, int n, const float* vector)
{
    GL_CHECK(glUniform4fv(location, n, vector));
}

void GL2Shader::loadBoolean(int location, bool b)
{
    float toload = 0.0f;
    if (b)
        toload = 1.0f;
    GL_CHECK(glUniform1f(location, toload));
}

void GL2Shader::loadMatrix4f(int location, const float* matrix)
{
    GL_CHECK(glUniformMatrix4fv(location, 1, false, matrix));
}

void GL2Shader::connectTexture(int location, int unitID)
{
    GL_CHECK(glUniform1i(location, unitID));
}

void GL2Shader::loadConfig(const UniformDefinition* ud, const void* data)
{
    switch (ud->type) {
    case GL_SAMPLER_2D: {
        const GL2Texture* texture = *static_cast<const GL2Texture* const*>(data);
        glActiveTexture(GL_TEXTURE0 + ud->location);
        glBindTexture(GL_TEXTURE_2D, texture->getTextureID());
        connectTexture(ud->location, ud->location);
    } break;

    case GL_FLOAT_VEC2:
        GL_CHECK(glUniform2fv(ud->location, ud->size, static_cast<const GLfloat*>(data)));
        break;

    case GL_FLOAT_VEC3:
        GL_CHECK(glUniform3fv(ud->location, ud->size, static_cast<const GLfloat*>(data)));
        break;

    case GL_FLOAT_VEC4:
        GL_CHECK(glUniform4fv(ud->location, ud->size, static_cast<const GLfloat*>(data)));
        break;

    case GL_FLOAT_MAT3:
        GL_CHECK(glUniformMatrix3fv(ud->location, ud->size, ud->extraInfo, static_cast<const GLfloat*>(data)));
        break;

    case GL_FLOAT_MAT4:
        GL_CHECK(glUniformMatrix4fv(ud->location, ud->size, ud->extraInfo, static_cast<const GLfloat*>(data)));
        break;

    default:
        LOG(EngineLog, BE_LOG_WARNING) << ud->type << " not handled.";
        break;
    }
}

GLuint GL2Shader::attachSource(GLint type, const void* data, GLint size)
{
    std::string error;
    int err = BE_NO_ERROR;

    GLuint out;
    err = compile(type, data, size, out, error);

    if (err != BE_NO_ERROR) {
        LOG(EngineLog, BE_LOG_ERROR) << "Shader (" << type << ") error: " << err << " - " << error;
    }

    pieces[npieces++] = out;

    return out;
}
}
