#include "Camera3DProcessor.h"

namespace BitEngine{

	Camera3DProcessor::Camera3DProcessor(EntitySystem* es, Transform3DProcessor* t2p)
		: m_entitySys(es), m_t2p(t2p), activeCamera(0)
	{
	}

	Camera3DComponent* Camera3DProcessor::getActiveCamera() const
	{
		return activeCameraRef; 
	}

	bool Camera3DProcessor::Init(){
		return true;
	}

	void Camera3DProcessor::recalculateViewMatrix(Camera3DComponent* c, Transform3DComponent* t)
	{
		glm::vec3 eye(t->getMatrix()[3][0], t->getMatrix()[3][1], t->getMatrix()[3][2]);
		//glm::vec3 eye = t->getPosition();
		c->m_viewMatrix = lookAt(eye, c->m_lookAt, c->m_up);
	}

	void Camera3DProcessor::FrameEnd()
	{
		Camera3DComponent* cam = components.getComponent(activeCamera);
			
		std::vector<ComponentHandle> search;
		std::vector<ComponentHandle> answer;
		std::vector<uint32> indices;

		search.push_back(activeCamera);
		m_entitySys->findAllTuples<Camera3DComponent, Transform3DComponent>(search, answer, indices);
		if (indices.size() == 1)
		{
			Transform3DComponent* cp = static_cast<Transform3DComponent*>(m_t2p->getComponent(answer[0]));

			recalculateViewMatrix(cam, cp);
			// printf("View matrix recalculated\n");
		}

		if (activeCamera != 0){
			activeCameraRef = components.getComponent(activeCamera);
		} else {
			activeCameraRef = nullptr;
		}
	}

	ComponentHandle Camera3DProcessor::CreateComponent(EntityHandle entity)
	{
		ComponentHandle c = components.newComponent();

		if (activeCamera == 0)
			activeCamera = c;

		return c;
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