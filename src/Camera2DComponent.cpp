#include "Camera2DComponent.h"


namespace BitEngine{

	Camera2DComponent::Camera2DComponent()
		: changed(true), m_zoom(1.0f)
	{
		
	}

	void Camera2DComponent::setPosition(const glm::vec3& pos)
	{
		m_position = pos;

		changed = true;
	}

	glm::vec3 Camera2DComponent::getPosition() const
	{
		return m_position;
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

	void Camera2DComponent::setActive(bool a)
	{
		m_active = a;
	}

	const glm::mat4& Camera2DComponent::getMatrix() const
	{
		return m_cameraMatrix;
	}

	void Camera2DComponent::recalculateMatrix()
	{
		changed = false;

		// Uses look at as focal point
		glm::vec3 translation = -m_position + glm::vec3(m_width / 2 - m_lookAt.x, m_height / 2 - m_lookAt.y, 0.0f);
		glm::vec3 scale(m_zoom, m_zoom, 0.0f);

		m_cameraMatrix = glm::translate(m_orthoMatrix, translation);

		// View * T * [R] * S
		m_cameraMatrix = glm::scale(glm::mat4(1.0f), scale) * m_cameraMatrix;
	}


}