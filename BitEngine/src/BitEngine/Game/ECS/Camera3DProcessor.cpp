#include "BitEngine/Game/ECS/Camera3DProcessor.h"

namespace BitEngine {

Camera3DProcessor::Camera3DProcessor(EntitySystem* es, Transform3DProcessor* t3dp)
    : ComponentProcessor(es), transform3DProcessor(t3dp)
{
}

void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent& c, const glm::mat4& modelMat)
{
    glm::vec3 eye(modelMat[3][0], modelMat[3][1], modelMat[3][2]);
    //glm::vec3 eye = t->getPosition();
    c.m_viewMatrix = glm::lookAt(eye, c.m_lookAt, c.m_up);
}

void Camera3DProcessor::Process()
{
    BE_PROFILE_FUNCTION();
    getES()->forEach<Camera3DComponent, Transform3DComponent>(
        [this](ComponentRef<Camera3DComponent>&& camera, ComponentRef<Transform3DComponent>&& transform)
    {
        recalculateViewMatrix(*camera, transform3DProcessor->getGlobalTransformFor(getComponentHandle(transform)));
    });

}

}
