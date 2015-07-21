#include "Transform2DComponent.h"

#include <math.h>

namespace BitEngine{

Transform2DComponent::Transform2DComponent()
	: m_dirty(DIRTY_ALL), m_nParents(0), parent(nullptr), scale(1.0f, 1.0f), rotation(0.0f)
{
}


Transform2DComponent::~Transform2DComponent()
{
}

const glm::vec2& Transform2DComponent::getPosition() const
{
	return position;
}

void Transform2DComponent::setPosition(const glm::vec2& p)
{
	position = p;
	m_dirty |= DIRTY_DATA;
}

float Transform2DComponent::getRotation() const
{
	return rotation;
}

void Transform2DComponent::setRotation(float rad)
{
	rotation = rad;
}

const glm::mat3& Transform2DComponent::getMatrix() const{
	return m_modelMatrix;
}

Transform2DComponent* Transform2DComponent::getParent() const
{ 
	return parent; 
}

void Transform2DComponent::setParent(Transform2DComponent* p){
	parent = p;
	m_dirty |= DIRTY_PARENT | DIRTY_DATA;
}

void Transform2DComponent::recalculateMatrix()
{
	// T R S
	float cosx = cos(rotation);
	float sinx = sin(rotation);
	m_modelMatrix = glm::transpose( glm::mat3(scale.x*cosx, scale.y*sinx, position.x,
								   -scale.x*sinx, scale.y*cosx, position.y,
								    0.0f, 0.0f, 1.0f));
	if (parent){
		m_modelMatrix = m_modelMatrix * parent->getMatrix();
	}
}



}