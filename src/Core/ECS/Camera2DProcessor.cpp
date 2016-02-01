#include "Core/ECS/Camera2DProcessor.h"

namespace BitEngine{

Camera2DProcessor::Camera2DProcessor()
{
}

bool Camera2DProcessor::Init(BaseEntitySystem* es)
{
	baseES = es;
	Transform2DType = baseES->getComponentType<Transform2DComponent>();
	Camera2DType = baseES->getComponentType<Camera2DComponent>();

	holderTransform = baseES->getHolder(Transform2DType);
	holderCamera = baseES->getHolder(Camera2DType);

	holderTransform->RegisterListener(this);
	holderCamera->RegisterListener(this);

	return true;
}

void Camera2DProcessor::Stop()
{
	holderCamera->UnregisterListener(this);
	holderTransform->UnregisterListener(this);
}

void Camera2DProcessor::OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
{
	if (holderCamera->getComponentHandleFor(entity) && holderTransform->getComponentHandleFor(entity)) {
		processEntities.emplace_back(entity);
	}
}

void Camera2DProcessor::OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
{

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

void Camera2DProcessor::Process()
{
	for (EntityHandle entity : processEntities)
	{
		Camera2DComponent* cam = (Camera2DComponent*)holderCamera->getComponentFor(entity);
		const Transform2DComponent* camTransform = (const Transform2DComponent*)holderTransform->getComponentFor(entity);
		if (cam->changed)
		{
			recalculateMatrix(cam, camTransform);
			cam->changed = false;
		}
	}
}

ComponentHandle Camera2DProcessor::AllocComponent()
{
	return components.newComponent();
}

void Camera2DProcessor::DeallocComponent(ComponentHandle component)
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
