#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Component.h"

#include "ComponentsType.h"

namespace BitEngine{

	class Camera3DComponent : public Component
	{
	public:
		enum ProjectionMode{
			ORTHOGONAL = 1,
			PERSPECTIVE,

		};

		static ComponentType getComponentType(){
			return COMPONENT_TYPE_CAMERA3D;
		}

		Camera3DComponent()
			: m_projectionType(PERSPECTIVE),
			m_fov(45.0f), m_aspect(16.0f / 9.0f), m_near(1.0f), m_far(1000.0f),
			m_lookAt(0.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f)
		{
			recalculateProjection();
		}

		~Camera3DComponent(){}

		void setView(int width, int height);

		void setFOV(float fov);
		void setNearFar(float near, float far);
		void setLookAt(const glm::vec3& lookAt);
		void setUp(const glm::vec3& up);

		
		const glm::mat4& getProjection() const { return m_projection; }
		const glm::mat4& getView() const { return m_viewMatrix; }

	private:
		friend class Camera3DProcessor;

		ProjectionMode m_projectionType;
		float m_width;
		float m_height;
		float m_fov;
		float m_aspect;
		float m_near;
		float m_far;

		glm::vec3 m_lookAt;
		glm::vec3 m_up;

		glm::mat4 m_viewMatrix;
		glm::mat4 m_projection;

		void recalculateProjection()
		{
			if (m_projectionType == PERSPECTIVE){
				m_projection = glm::perspective(m_fov, m_aspect, m_near, m_far);
			}
			else {
				m_projection = glm::ortho(0.0f, m_width, 0.0f, m_height, m_near, m_far);
			}
		}
	};


}