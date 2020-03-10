#include "BitEngine/Game/ECS/Camera3DComponent.h"

namespace BitEngine{

	void Camera3DComponent::recalculateProjection()
	{
		if (m_projectionType == PERSPECTIVE) {
			m_projection = glm::perspective(m_fov, m_aspect, m_near, m_far);
		}
		else {
			m_projection = glm::ortho(0.0f, (float)m_width, 0.0f, (float)m_height, m_near, m_far);
		}
	}

}