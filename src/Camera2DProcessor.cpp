#include "Camera2DProcessor.h"

namespace BitEngine{

Camera2DProcessor::Camera2DProcessor(EntitySystem* es, Transform2DProcessor* t2p)
	: m_entitySys(es), m_t2p(t2p), activeCamera(nullptr)
{
}

Camera2DComponent* Camera2DProcessor::getActiveCamera()
{
	return activeCamera;
}

bool Camera2DProcessor::Init(){
	return true;
}

void Camera2DProcessor::recalculateMatrix(Camera2DComponent* c, Transform2DComponent* t)
{
	// Uses look at as focal point
	glm::vec2 translation = -t->getPosition() + glm::vec2(c->m_width / 2 - c->m_lookAt.x, c->m_height / 2 - c->m_lookAt.y);
	glm::vec3 scale(c->m_zoom, c->m_zoom, 0.0f);

	c->m_cameraMatrix = glm::translate(c->m_orthoMatrix, glm::vec3(translation,0));

	// View * T * [R] * S
	c->m_cameraMatrix = glm::scale(glm::mat4(1.0f), scale) * c->m_cameraMatrix;
}

void Camera2DProcessor::FrameEnd()
{
	std::vector<ComponentHandle>& cameras = components.getValidComponents();

	for (ComponentHandle c : cameras)
	{
		Camera2DComponent* cam = components.getComponent(c);
		if (cam->m_active)
		{
			std::vector<ComponentHandle> search;
			std::vector<ComponentHandle> answer;
			std::vector<uint32> indices;
			search.push_back(c);
			
			m_entitySys->findAllTuples<Camera2DComponent, Transform2DComponent>(search, answer, indices);
			if (indices.size() == 1)
			{
				Transform2DComponent* cp = static_cast<Transform2DComponent*>(m_t2p->getComponent(answer[0]));

				activeCamera = cam;
				recalculateMatrix(cam, cp);
				break;
			}
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

std::vector<ComponentHandle>& Camera2DProcessor::getComponents()
{
	return components.getValidComponents();
}

}