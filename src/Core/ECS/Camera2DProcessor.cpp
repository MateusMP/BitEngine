#include "Core/ECS/Camera2DProcessor.h"

namespace BitEngine{

	Camera2DProcessor::Camera2DProcessor(Transform2DProcessor* t2dp)
		: transform2DProcessor(t2dp)
	{
	}

	bool Camera2DProcessor::Init()
	{
		Transform2DType = Transform2DComponent::getComponentType(); // baseES->getComponentType<Transform2DComponent>();
		Camera2DType = Camera2DComponent::getComponentType();		// baseES->getComponentType<Camera2DComponent>();

		holderTransform = getES()->getHolder<Transform2DComponent>();
		holderCamera = getES()->getHolder<Camera2DComponent>();

		return true;
	}

	void Camera2DProcessor::Stop()
	{
	}

	void Camera2DProcessor::OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		if (holderCamera->getComponentForEntity(entity) && holderTransform->getComponentForEntity(entity)) {
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
		getES()->forEach<Camera2DComponent, Transform2DComponent>(
            [](EntityHandle entity, Camera2DComponent& camera, const Transform2DComponent& transform)
            {
                // ComponentHandle componentID = holderCamera->getComponentForEntity(entity);
                // Camera2DComponent* cam = holderCamera->getComponent(componentID);
                // componentID = holderTransform->getComponentForEntity(entity);
                // const Transform2DComponent* camTransform = holderTransform->getComponent(componentID);
                if (camera.changed)
                {
                    recalculateMatrix(&camera, &transform);
                    camera.changed = false;
                }

            }
        );
	}

}
