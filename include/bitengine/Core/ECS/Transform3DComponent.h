#pragma once

#include <glm\common.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "bitengine/Core/ECS/Component.h"

namespace BitEngine{


	class Transform3DComponent :
		public Component<Transform3DComponent>
	{
	public:
		const static glm::vec3 FORWARD;
		const static glm::vec3 UP;
		const static glm::vec3 RIGHT;

		Transform3DComponent();
		~Transform3DComponent();

		// Position
		const glm::vec3& getPosition() const;
		template<typename T>
		void setPosition(T x, T y, T z)
		{
			position.x = (float)x;
			position.y = (float)y;
			position.z = (float)z;
			m_dirty = true;
		}
		void setPosition(const glm::vec3& p);

		// Scale
		const glm::vec3& getScale() const;
		void setScale(const glm::vec3& s);

		// Rotation
		const glm::quat& getRotation() const;
		void setRotation(const glm::quat& quat);

		const glm::vec3 getForward() const {
			return rotation * FORWARD;
		}

		const glm::vec3 getRight() const {
			return rotation * RIGHT;
		}


	private:
		friend class Transform3DProcessor;

		glm::vec3 position;
		glm::vec3 scale;
		glm::quat rotation;

		bool m_dirty;
	};


}
