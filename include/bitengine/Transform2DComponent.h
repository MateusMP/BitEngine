#pragma once

#include <glm\common.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "Component.h"
#include "ComponentsType.h"

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

	static ComponentType getComponentType(){
		return COMPONENT_TYPE_TRANSFORM2D;
	}


	Transform2DComponent();
	~Transform2DComponent();

	// Position
	const glm::vec2& getPosition() const;
	template<typename T>
	void setPosition(T x, T y)
	{
		position.x = (float)x;
		position.y = (float)y;
		m_dirty |= DIRTY_DATA;
	}
	void setPosition(const glm::vec2& p);

	// Scale
	const glm::vec2& getScale() const;
	void setScale(const glm::vec2& s);

	// Rotation
	float getRotation() const;
	void setRotation(float rad);

	/**
	 * Retrieves the last calculated model matrix
	 * This is updated by Transform2DProcessor
	 */
	const glm::mat3& getMatrix() const;

	void setParent(ComponentHandle p);
	ComponentHandle getParent() const;


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
