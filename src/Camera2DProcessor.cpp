#include "Camera2DProcessor.h"


namespace BitEngine{

Camera2DProcessor::Camera2DProcessor()
	: activeCamera(nullptr)
{
}

Camera2DComponent* Camera2DProcessor::getActiveCamera()
{
	return activeCamera;
}

bool Camera2DProcessor::Init(){
	return true;
}

void Camera2DProcessor::Process()
{
	std::vector<Camera2DComponent*>& cameras = components.getValidComponents();
	for (Camera2DComponent* c : cameras)
	{
		if (c->m_active){
			activeCamera = c;
			activeCamera->recalculateMatrix();
			break;
		}
	}
}

ComponentHandle Camera2DProcessor::CreateComponent()
{
	return components.newComponent();
}

void Camera2DProcessor::DestroyComponent(ComponentHandle component)
{
	components.removeComponent(component);
}

Component* Camera2DProcessor::getComponent(ComponentHandle hdl)
{
	return components.getComponent(hdl);
}

std::vector<Camera2DComponent*>& Camera2DProcessor::getComponents()
{
	return components.getValidComponents();
}

}