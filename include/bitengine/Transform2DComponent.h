#pragma once

#include <glm\common.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Component.h"

namespace BitEngine{

class Transform2DComponent :
	public Component
{
public:
	enum Dirty : unsigned char{
		DIRTY_PARENT = 1,
		DIRTY_DATA = 2,
		DIRTY_ALL = DIRTY_PARENT | DIRTY_DATA
	};

	Transform2DComponent();
	~Transform2DComponent();

	// Position
	const glm::vec2& getLocalPosition() const; // returns LOCAL position
	template<typename T>
	void setLocalPosition(T x, T y) // sets LOCAL position
	{
		position.x = (float)x;
		position.y = (float)y;
		m_dirty |= DIRTY_DATA;
	}
	void setLocalPosition(const glm::vec2& p); // sets LOCAL position

	// Scale
	const glm::vec2& getLocalScale() const; // get LOCAL scale
	void setLocalScale(const glm::vec2& s); // sets LOCAL scale

	// Rotation
	float getLocalRotation() const; // get LOCAL rotation
	void setLocalRotation(float rad); // sets LOCAL rotation

	/**
	 * Retrieves the last calculated model matrix
	 * This is updated by Transform2DProcessor
	 */
	const glm::mat3& getMatrix() const;

	void setParent(ComponentHandle p);
	ComponentHandle getParent() const;

	float getWorldAngle() const {
		return atan2(m_modelMatrix[0][1], m_modelMatrix[1][1]);
	}

	glm::vec2 getRight() const {
		return glm::mat2(m_modelMatrix) * glm::vec2(1, 0);
	}

private:
	friend class Transform2DProcessor;

	glm::vec2 position;
	glm::vec2 scale;
	float rotation;

	char m_dirty;
	int m_nParents;
	ComponentHandle parent;

	glm::mat3 m_modelMatrix;
};


}
