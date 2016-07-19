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
            [](ComponentRef<Camera2DComponent>& camera, const ComponentRef<Transform2DComponent>& transform)
            {
                if (camera->changed)
                {
                    recalculateMatrix(&camera.ref(), &transform.ref());
                    camera->changed = false;
                }
            }
        );
	}

}
