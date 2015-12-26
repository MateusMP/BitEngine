#include "Camera3DProcessor.h"

namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor()
	{
	}

	bool Camera3DProcessor::Init(BaseEntitySystem* es){

		baseES = es;
		Transform3DType = baseES->getComponentType<Transform3DComponent>();
		Camera3DType = baseES->getComponentType<Camera3DComponent>();

		holderCamera = baseES->getHolder(Camera3DType);
		holderTransform = baseES->getHolder(Transform3DType);

		holderCamera->RegisterListener(this);
		holderTransform->RegisterListener(this);

		return true;
	}

	void Camera3DProcessor::Stop() {

	}

	void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent* c, const Transform3DComponent* t)
	{
		glm::vec3 eye(t->getMatrix()[3][0], t->getMatrix()[3][1], t->getMatrix()[3][2]);
		//glm::vec3 eye = t->getPosition();
		c->m_viewMatrix = glm::lookAt(eye, c->m_lookAt, c->m_up);
	}

	void Camera3DProcessor::Process()
	{
		std::vector<Component*> answer;
		// m_entitySys->findAllTuplesOf<Camera3DComponent, Transform3DComponent>(answer);
		
		for (EntityHandle entity : processEntities)
		{
			Camera3DComponent* cam = (Camera3DComponent*)holderCamera->getComponentFor(entity);
			const Transform3DComponent* camTransform = (const Transform3DComponent*)holderTransform->getComponentFor(entity);
			
			recalculateViewMatrix(cam, camTransform);
		}
	}

	ComponentHandle Camera3DProcessor::AllocComponent()
	{
		return components.newComponent();
	}

	void Camera3DProcessor::DeallocComponent(ComponentHandle component)
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