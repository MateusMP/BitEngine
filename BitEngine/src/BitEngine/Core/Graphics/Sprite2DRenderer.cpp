#include "BitEngine/Core/Graphics/Sprite2DRenderer.h"

namespace BitEngine {

// TODO: Make these dynamic and available in a library
Material sprite_materials[3];
const Material* Sprite2DComponent::DEFAULT_SPRITE = &sprite_materials[0];
const Material* Sprite2DComponent::TRANSPARENT_SPRITE = &sprite_materials[1];
const Material* Sprite2DComponent::EFFECT_SPRITE = &sprite_materials[2];


Sprite2DRenderer::Sprite2DRenderer(EntitySystem* es, ResourceLoader* resourceLoader)
    : ComponentProcessor(es), m_batch(nullptr), resourceLoader(resourceLoader)
{
    Init();
}

void Sprite2DRenderer::setActiveCamera(ComponentRef<Camera2DComponent>& handle)
{
    activeCamera = handle;
}

bool Sprite2DRenderer::Init()
{
    // DEFAULT_SPRITE
    sprite_materials[0].setState(RenderConfig::BLEND, BlendConfig::BLEND_NONE);
    sprite_materials[0].setState(RenderConfig::TEXTURE_2D, true);
    sprite_materials[0].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    // TRANSPARENT_SPRITE
    sprite_materials[1].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
    sprite_materials[1].setState(RenderConfig::TEXTURE_2D, true);
    sprite_materials[1].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);
    sprite_materials[1].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    // EFFECT_SPRITE
    sprite_materials[2].setState(RenderConfig::BLEND, BlendConfig::BLEND_ALL);
    sprite_materials[2].setState(RenderConfig::TEXTURE_2D, true);
    sprite_materials[2].setState(RenderConfig::DEPTH_TEST, DepthConfig::DEPTH_TEST_DISABLED);
    sprite_materials[2].setBlendEquation(BlendEquation::ADD);
    sprite_materials[2].setBlendMode(BlendFunc::SRC_ALPHA, BlendFunc::ONE_MINUS_SRC_ALPHA);

    const char* SPRITE_2D_SHADER_PATH = "sprite2Dshader";
    shader = resourceLoader->getResource<Shader>(SPRITE_2D_SHADER_PATH);
    if (!shader.isValid()) {
        LOG(BitEngine::EngineLog, BE_LOG_ERROR) << "Failed to load sprite 2D shader: " << SPRITE_2D_SHADER_PATH;
        return false;
    }

    newRefs.init(shader.get());
    m_batch = shader->createBatch();

    return true;
}

void Sprite2DRenderer::Stop()
{
    delete m_batch;
}

const std::vector<Sprite2DBatch>& Sprite2DRenderer::GenerateRenderData()
{
    buildBatchInstances();   
    return batches;
}

void Sprite2DRenderer::Render() {
 
    if (m_batch == nullptr)
    {
        if (!shader->isReady()) {
            LOG(BitEngine::EngineLog, BE_LOG_WARNING) << "Skipping rendering until shader is loaded";
            return;
        }
        else {
            m_batch = shader->createBatch();
        }
    }

    m_batch->clear();

    for (auto& it : batches) {
        prepare_new(it);
        m_batch->load();
        m_batch->render(shader.get());
    }
}

void Sprite2DRenderer::buildBatchInstances()
{
    if (!activeCamera) { return; }

    for (Sprite2DBatch& it : batches) {
        it.batchInstances.clear();
    }

    const glm::vec4 viewScreen = activeCamera->getWorldViewArea();

    // Build batch
    getES()->forEach<SceneTransform2DComponent, Sprite2DComponent>(
        [=](const ComponentRef<SceneTransform2DComponent>& transform, const ComponentRef<Sprite2DComponent>& sprite)
    {
        if (insideScreen(viewScreen, transform->getGlobal(), 64))
        {
            const Texture* texture = sprite->sprite->getTexture().get();
            Sprite2DBatch::BatchIdentifier idtf(sprite->layer, sprite->material, sprite->sprite->getTexture().get());
            const auto& it = batchesMap.find(idtf);
            if (it != batchesMap.end()) {
                batches[it->second].batchInstances.emplace_back(transform.ref(), sprite.ref());
            }
            else {
                batches.emplace_back(idtf);
                size_t id = batches.size() - 1;
                batchesMap.emplace(idtf, id);
                batches[id].bid = idtf;
                batches[id].batchInstances.emplace_back(transform.ref(), sprite.ref());
            }
        }
    });
}

void Sprite2DRenderer::prepare_new(Sprite2DBatch& batch)
{
    std::vector<Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;

    Sprite2D_DD_new::CamMatricesContainer* view = m_batch->getShaderDataAs<Sprite2D_DD_new::CamMatricesContainer>(newRefs.u_viewMatrixContainer);
    if (view) {
        view->view = activeCamera->getMatrix();
    }

    m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
    // Prepare for all instances
    m_batch->prepare(batchInstances.size() * 6);

    // Setup batch data
    if (!batchInstances.empty())
    {
        Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(newRefs.m_textureContainer);
        texture->diffuse = batch.bid.texture;

        Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(newRefs.u_modelMatrixContainer);
        Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(newRefs.m_ptnContainer);

        const glm::vec2 vertex_pos[4] = {
                glm::vec2(0.0f, 0.0f),
                glm::vec2(1.0f, 0.0f),
                glm::vec2(0.0f, 1.0f),
                glm::vec2(1.0f, 1.0f)
        };
        const size_t instanceCount = batchInstances.size();
        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];

            modelMatrices[i].modelMatrix = inst.transform.m_global;

            const glm::vec2 sizes(inst.sprite.sprite->getWidth(), inst.sprite.sprite->getHeight());
            const glm::vec2 offsets(-inst.sprite.sprite->getOffsetX(), -inst.sprite.sprite->getOffsetY());
            const glm::vec2 off_siz = offsets * sizes;
            
            vertices[i*6 + 0].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[0] * sizes + off_siz, 1));
            vertices[i*6 + 1].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
            vertices[i*6 + 2].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
            vertices[i*6 + 3].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[2] * sizes + off_siz, 1));
            vertices[i*6 + 4].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[1] * sizes + off_siz, 1));
            vertices[i*6 + 5].position = glm::vec2(inst.transform.m_global * glm::vec3(vertex_pos[3] * sizes + off_siz, 1));
            
            const glm::vec4& uvrect = inst.sprite.sprite->getUV();
            vertices[i*6 + 0].textureUV = glm::vec2(uvrect.x, uvrect.y); // BL  xw   zw glm::vec2(0, 1);
            vertices[i*6 + 1].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
            vertices[i*6 + 2].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(1, 0);
            vertices[i*6 + 3].textureUV = glm::vec2(uvrect.x, uvrect.w); // TL             glm::vec2(0, 1);
            vertices[i*6 + 4].textureUV = glm::vec2(uvrect.z, uvrect.y); // BR             glm::vec2(0, 0);
            vertices[i*6 + 5].textureUV = glm::vec2(uvrect.z, uvrect.w); // TR  xy   zy glm::vec2(1, 1);
        }
    }
}

/*
void Sprite2DRenderer::prepare_instanced(Sprite2DBatch& batch)
{
    std::vector<Sprite2DBatch::SpriteBatchInstance>& batchInstances = batch.batchInstances;

    m_batch->setVertexRenderMode(VertexRenderMode::TRIANGLE_STRIP);
    // Prepare for all instances
    m_batch->prepare(batchInstances.size());

    // Setup batch data
    if (!batchInstances.empty())
    {
        Sprite2D_DD_new::TextureContainer* texture = m_batch->getShaderDataAs<Sprite2D_DD_new::TextureContainer>(newRefs.m_textureContainer);
        texture->diffuse = batch.bid.texture;

        Sprite2D_DD_new::ModelMatrixContainer* modelMatrices = m_batch->getShaderDataAs<Sprite2D_DD_new::ModelMatrixContainer>(newRefs.u_modelMatrixContainer);
        Sprite2D_DD_new::PTNContainer* vertices = m_batch->getShaderDataAs<Sprite2D_DD_new::PTNContainer>(newRefs.m_ptnContainer);

        const size_t instanceCount = batchInstances.size();
        for (size_t i = 0; i < instanceCount; ++i)
        {
            const Sprite2DBatch::SpriteBatchInstance& inst = batchInstances[i];

            modelMatrices[i].modelMatrix = inst.transform.m_global;

            vertices[i].textureUV = inst.sprite.sprite->getUV();
        }
    }
}*/

}