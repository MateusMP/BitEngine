#pragma once

#include <glm\common.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Component.h"

namespace BitEngine{


	class Transform3DComponent :
		public Component
	{
	public:
		const static glm::vec3 FORWARD;
		const static glm::vec3 UP;
		const static glm::vec3 RIGHT;

		enum Dirty : unsigned char{
			DIRTY_PARENT = 1,
			DIRTY_DATA = 2,
			DIRTY_ALL = DIRTY_PARENT | DIRTY_DATA
		};

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
			m_dirty |= DIRTY_DATA;
		}
		void setPosition(const glm::vec3& p);

		// Scale
		const glm::vec3& getScale() const;
		void setScale(const glm::vec3& s);

		// Rotation
		const glm::quat& getRotation() const;
		void setRotation(const glm::quat& quat);

		/**
		* Retrieves the last calculated model matrix
		* This is updated by Transform2DProcessor
		*/
		const glm::mat4& getMatrix() const;

		void setParent(ComponentHandle p);
		ComponentHandle getParent() const;

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

		char m_dirty;
		int m_nParents;
		ComponentHandle parent;

		glm::mat4 m_modelMatrix;
	};


}
