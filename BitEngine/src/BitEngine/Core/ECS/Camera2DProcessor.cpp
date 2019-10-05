#include "bitengine/Core/ECS/Camera2DProcessor.h"

namespace BitEngine{

	Camera2DProcessor::Camera2DProcessor(Transform2DProcessor* t2dp)
		: ComponentProcessor(t2dp->getMessenger()), transform2DProcessor(t2dp)
	{
	}

	bool Camera2DProcessor::Init()
	{
		Camera2DType = Camera2DComponent::getComponentType();		// baseES->getComponentType<Camera2DComponent>();

		holderTransform = getES()->getHolder<Transform2DComponent>();
		holderCamera = getES()->getHolder<Camera2DComponent>();

		return true;
	}

	void Camera2DProcessor::Stop()
	{
	}

	void Camera2DProcessor::recalculateMatrix(Camera2DComponent& c)
	{
		// Uses look at as focal point
		const glm::vec2 focusOrigin(c.m_lookAt.x - c.m_width/2, c.m_lookAt.y - c.m_height / 2);
		const glm::vec2 translation = -focusOrigin;
		const glm::vec3 scale(c.m_zoom, c.m_zoom, 1.0f);
		const glm::mat4 transform = glm::scale(glm::translate(glm::mat4(1), glm::vec3(translation, 0)), scale);
		
		// View * T * [R] * S
		c.m_cameraMatrix = c.m_orthoMatrix * transform;
	}

	void Camera2DProcessor::Process()
	{
		//ComponentHandle handle, GameLogicComponent& l
		// ComponentRef<Camera2DComponent>& camera
		getES()->forAll<Camera2DComponent>(
            [](ComponentHandle handle, Camera2DComponent& camera)
            {
                if (camera.changed)
                {
                    recalculateMatrix(camera);
                    camera.changed = false;
                }
            }
        );
	}

}