#include "Transform2DComponent.h"

#include <math.h>

namespace BitEngine{

Transform2DComponent::Transform2DComponent()
	: m_dirty(DIRTY_ALL), m_nParents(0), parent(0), scale(1.0f, 1.0f), rotation(0.0f)
{
}


Transform2DComponent::~Transform2DComponent()
{
}

const glm::vec2& Transform2DComponent::getLocalPosition() const
{
	return position;
}

void Transform2DComponent::setLocalPosition(const glm::vec2& p)
{
	position = p;
	m_dirty |= DIRTY_DATA;
}

const glm::vec2& Transform2DComponent::getLocalScale() const
{
	return scale;
}

void Transform2DComponent::setLocalScale(const glm::vec2& s)
{
	scale = s;
	m_dirty |= DIRTY_DATA;
}

float Transform2DComponent::getLocalRotation() const
{
	return rotation;
}

void Transform2DComponent::setLocalRotation(float rad)
{
	rotation = rad;
	m_dirty |= DIRTY_DATA;
}

const glm::mat3& Transform2DComponent::getMatrix() const{
	return m_modelMatrix;
}

ComponentHandle Transform2DComponent::getParent() const
{ 
	return parent; 
}

void Transform2DComponent::setParent(ComponentHandle h){
	parent = h;
	m_dirty |= DIRTY_PARENT | DIRTY_DATA;
}


}