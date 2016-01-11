#include "Camera3DProcessor.h"

namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor()
	{
	}

	bool Camera3DProcessor::Init(BaseEntitySystem* es){

		baseES = es;
		Transform3DType = baseES->getComponentType<Transform3DComponent>();
		Camera3DType = baseES->getComponentType<Camera3DComponent>();

		holderCamera = (Camera3DProcessor*) baseES->getHolder(Camera3DType);
		holderTransform = (Transform3DProcessor*)baseES->getHolder(Transform3DType);

		holderCamera->RegisterListener(this);
		holderTransform->RegisterListener(this);

		return true;
	}

	void Camera3DProcessor::Stop() {

	}


	void Camera3DProcessor::OnComponentCreated(EntityHandle entity, ComponentType type, ComponentHandle component)
	{
		ComponentHandle camera = holderCamera->getComponentHandleFor(entity);
		ComponentHandle transform = holderTransform->getComponentHandleFor(entity);
		
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
			
			recalculateViewMatrix(cam, holderTransform->getGlobalTransformFor(entry.transform3d) );
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