#pragma once

#include "Component.h"

#include <glm\common.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include "EntitySystem.h"

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

	const glm::vec2& getPosition() const;
	template<typename T>
	void setPosition(T x, T y)
	{
		position.x = (float)x;
		position.y = (float)y;
		m_dirty |= DIRTY_DATA;
	}
	void setPosition(const glm::vec2& p);

	const glm::mat3& getMatrix() const;

	void setParent(Transform2DComponent* p);
	Transform2DComponent* getParent() const;


private:
	friend class Transform2DProcessor;
	void recalculateMatrix();

	glm::vec2 position;
	glm::vec2 scale;
	float rotation;

	char m_dirty;
	int m_nParents;
	Transform2DComponent* parent;

	glm::mat3 m_modelMatrix;
};


}
