#include "BitEngine/Core/ECS/Camera3DProcessor.h"

#include <tuple>
namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor(EntitySystem* es, Transform3DProcessor* t3dp)
		: ComponentProcessor(es), transform3DProcessor(t3dp)
	{
		Transform3DType = Transform3DComponent::getComponentType();	// baseES->getComponentType<Transform3DComponent>();
		Camera3DType = Camera3DComponent::getComponentType();	// baseES->getComponentType<Camera3DComponent>();

		holderTransform = getES()->getHolder<Transform3DComponent>();
		holderCamera = getES()->getHolder<Camera3DComponent>();
	}
    
	void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent& c, const glm::mat4& modelMat)
	{
		glm::vec3 eye(modelMat[3][0], modelMat[3][1], modelMat[3][2]);
		//glm::vec3 eye = t->getPosition();
		c.m_viewMatrix = glm::lookAt(eye, c.m_lookAt, c.m_up);
	}

	void Camera3DProcessor::Process()
	{
		/*
		getES()->forEachRef<Camera3DProcessor, Camera3DComponent, Transform3DComponent>(*this,
				[](Camera3DProcessor& self, EntityHandle entity, Camera3DComponent& camera, ComponentRef<Transform3DComponent>&& transform)
			{
				recalculateViewMatrix(camera, self.transform3DProcessor->getGlobalTransformFor(getComponentHandle(transform)));
			}
		);
		*/
	}

}
