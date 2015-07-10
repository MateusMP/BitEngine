

#include "Camera2D.h"


namespace BitEngine{

	Camera2D::Camera2D()
		: changed(true), m_zoom(1.0f)
	{
		
	}

	void Camera2D::setPosition(const glm::vec3& pos)
	{
		m_position = pos;

		changed = true;
	}

	glm::vec3 Camera2D::getPosition() const
	{
		return m_position;
	}

	void Camera2D::setLookAt(const glm::vec3& pos)
	{
		m_lookAt = pos;

		changed = true;
	}

	glm::vec3 Camera2D::getLookAt() const
	{
		return m_lookAt;
	}

	void Camera2D::setView(int width, int height)
	{
		m_width = width;
		m_height = height;

		m_orthoMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);

		changed = true;
	}


	void Camera2D::setZoom(float z)
	{
		m_zoom = z;

		changed = true;
	}

	float Camera2D::getZoom() const
	{
		return m_zoom;
	}

	void Camera2D::Update()
	{
		if (changed)
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

	glm::mat4 Camera2D::getMatrix() const
	{
		return m_cameraMatrix;
	}


}