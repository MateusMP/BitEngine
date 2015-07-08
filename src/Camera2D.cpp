

#include "Camera2D.h"


namespace BitEngine{

	void Camera2D::setPosition(const glm::vec3& pos)
	{
		position = pos;
	}

	glm::vec3 Camera2D::getPosition() const
	{
		return m_position;
	}

	void Camera2D::setLookAt(const glm::vec3& pos)
	{
		m_lookAt = pos;
	}

	glm::vec3 Camera2D::getLookAt() const
	{
		return m_lookAt;
	}

	void Camera2D::setView(int width, int height)
	{
		m_width = width;
		m_height = height;

		m_orthoMatrix = glm::ortho(0, width, 0, height);
	}


	void Camera2D::setZoom(float z)
	{
		m_zoom = z;
	}

	float Camera2D::getZoom() const
	{
		return m_zoom;
	}

	void Camera2D::Update()
	{
		// Uses look at as focal point
		glm::vec3 translation = m_position + glm::vec3(m_lookAt.x + m_width / 2, lookAt.y + m_height / 2, 0.0f);

		m_cameraMatrix = glm::translate(m_orthoMatrix, translation);
		m_cameraMatrix = glm::scale(m_cameraMatrix, glm::vec3(m_zoom) );
	}

	glm::mat4 Camera2D::getMatrix() const
	{
		return m_cameraMatrix;
	}


}