#include "Core/ECS/Camera3DProcessor.h"

namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor(Transform3DProcessor* t3dp)
		: transform3DProcessor(t3dp)
	{
	}

	bool Camera3DProcessor::Init()
	{
		Transform3DType = Transform3DComponent::getComponentType();	// baseES->getComponentType<Transform3DComponent>();
		Camera3DType = Camera3DComponent::getComponentType();	// baseES->getComponentType<Camera3DComponent>();

		holderTransform = getES()->getHolder<Transform3DComponent>();
		holderCamera = getES()->getHolder<Camera3DComponent>();

		return true;
	}

	void Camera3DProcessor::Stop() {

	}


	void Camera3DProcessor::OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		ComponentHandle camera = holderCamera->getComponentForEntity(entity);
		ComponentHandle transform = holderTransform->getComponentForEntity(entity);

		if (camera && transform) {
			processEntries.emplace_back(entity, camera, transform);
		}
	}

	void Camera3DProcessor::OnComponentDestroyed(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		invalidatedEntries.emplace(entity);
	}

	void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent* c, const glm::mat4& modelMat)
	{
		glm::vec3 eye(modelMat[3][0], modelMat[3][1], modelMat[3][2]);
		//glm::vec3 eye = t->getPosition();
		c->m_viewMatrix = glm::lookAt(eye, c->m_lookAt, c->m_up);
	}

	void Camera3DProcessor::Process()
	{
		// Remove invalidated entries
		if (!invalidatedEntries.empty())
		{
			for (unsigned i = 0; i < processEntries.size(); ++i)
			{
				auto it = invalidatedEntries.find(processEntries[i].entity);
				if (it != invalidatedEntries.end())
				{
					processEntries[i] = processEntries.back();
					processEntries.pop_back();
					// invalidatedEntries.erase(it); // TODO: Delete at each iteration? or one single clear at the end?
					break;
				}
			}

			invalidatedEntries.clear();
		}

		for (const Entry& entry : processEntries)
		{
			Camera3DComponent* cam = (Camera3DComponent*)holderCamera->getComponent(entry.camera3d);

			recalculateViewMatrix(cam, transform3DProcessor->getGlobalTransformFor(entry.transform3d) );
		}
	}

}
