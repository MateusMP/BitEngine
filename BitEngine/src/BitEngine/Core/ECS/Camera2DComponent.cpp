#include "bitengine/Core/ECS/Camera2DComponent.h"

namespace BitEngine{
    
	Camera2DComponent::Camera2DComponent()
		: m_zoom(1.0f), changed(true)
	{
	}

	void Camera2DComponent::setLookAt(const glm::vec3& pos)
	{
		m_lookAt = pos;

		changed = true;
	}

	glm::vec3 Camera2DComponent::getLookAt() const
	{
		return m_lookAt;
	}

	void Camera2DComponent::setView(int width, int height)
	{
		m_width = width;
		m_height = height;

		m_orthoMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

		changed = true;
	}


	void Camera2DComponent::setZoom(float z)
	{
		m_zoom = z;

		changed = true;
	}

	float Camera2DComponent::getZoom() const
	{
		return m_zoom;
	}

	const glm::mat4& Camera2DComponent::getMatrix() const {
		return m_cameraMatrix;
	}

	glm::vec4 Camera2DComponent::getWorldViewArea() const{

		return glm::vec4(m_lookAt.x - m_width / 2,	m_lookAt.y - m_height / 2,
						 m_lookAt.x + m_width / 2,	m_lookAt.y + m_height / 2);
	}

	const glm::vec2 Camera2DComponent::screenToWorldCoordinates(const glm::vec2& screenCoord) const {
		return glm::vec2(m_lookAt.x - m_width / 2 + screenCoord.x,
						 m_lookAt.y - m_height / 2 + screenCoord.y);
	}

}
