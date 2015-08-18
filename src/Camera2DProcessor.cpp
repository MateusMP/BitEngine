#include "Camera2DProcessor.h"

namespace BitEngine{

Camera2DProcessor::Camera2DProcessor(EntitySystem* es)
	: m_entitySys(es)
{
}

bool Camera2DProcessor::Init(){
	return true;
}

void Camera2DProcessor::recalculateMatrix(Camera2DComponent* c, const Transform2DComponent* t)
{
	// Uses look at as focal point
	glm::vec2 translation = -t->getLocalPosition() + glm::vec2(c->m_width / 2 - c->m_lookAt.x, c->m_height / 2 - c->m_lookAt.y);
	glm::vec3 scale(c->m_zoom, c->m_zoom, 0.0f);

	c->m_cameraMatrix = glm::translate(c->m_orthoMatrix, glm::vec3(translation,0));

	// View * T * [R] * S
	c->m_cameraMatrix = glm::scale(glm::mat4(1.0f), scale) * c->m_cameraMatrix;
}

void Camera2DProcessor::FrameEnd()
{
	const std::vector<ComponentHandle>& cameras = components.getValidComponents();

	std::vector<Component*> answer;
	m_entitySys->findAllTuplesOf<Camera2DComponent, Transform2DComponent>(answer);

	for (uint32 i = 0; i < answer.size(); i+=2)
	{
		Camera2DComponent* cam = (Camera2DComponent*)answer[i];
		const Transform2DComponent* camTransform = (const Transform2DComponent*)answer[i + 1];
		if (cam->changed)
		{	
			recalculateMatrix(cam, camTransform);
		}
	}
}

ComponentHandle Camera2DProcessor::CreateComponent(EntityHandle entity)
{
	return components.newComponent();
}

void Camera2DProcessor::DestroyComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Camera2DProcessor::getComponent(ComponentHandle component)
{
	return components.getComponent(component);
}

const std::vector<ComponentHandle>& Camera2DProcessor::getComponents() const
{
	return components.getValidComponents();
}

}