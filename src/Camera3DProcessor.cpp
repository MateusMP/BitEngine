#include "Camera3DProcessor.h"

namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor(EntitySystem* es)
		: m_entitySys(es)
	{
	}

	bool Camera3DProcessor::Init(){
		return true;
	}

	void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent* c, const Transform3DComponent* t)
	{
		glm::vec3 eye(t->getMatrix()[3][0], t->getMatrix()[3][1], t->getMatrix()[3][2]);
		//glm::vec3 eye = t->getPosition();
		c->m_viewMatrix = glm::lookAt(eye, c->m_lookAt, c->m_up);
	}

	void Camera3DProcessor::FrameEnd()
	{
		std::vector<Component*> answer;
		m_entitySys->findAllTuplesOf<Camera3DComponent, Transform3DComponent>(answer);

		for (uint32 i = 0; i < answer.size(); i += 2)
		{
			Camera3DComponent* cam = (Camera3DComponent*)answer[i];
			const Transform3DComponent* camTransform = (const Transform3DComponent*)answer[i + 1];
			recalculateViewMatrix(cam, camTransform);
		}
	}

	ComponentHandle Camera3DProcessor::CreateComponent(EntityHandle entity)
	{
		return components.newComponent();
	}

	void Camera3DProcessor::DestroyComponent(ComponentHandle component)
	{
		components.removeComponent(component);
	}

	Component* Camera3DProcessor::getComponent(ComponentHandle component)
	{
		return components.getComponent(component);
	}

	const std::vector<ComponentHandle>& Camera3DProcessor::getComponents() const
	{
		return components.getValidComponents();
	}

}