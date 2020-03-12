#include "BitEngine/Game/ECS/Transform3DComponent.h"

namespace BitEngine{
	
	const glm::vec3 Transform3DComponent::FORWARD(0.0f, 0.0f, -1.0f);
	const glm::vec3 Transform3DComponent::UP(0.0f, 1.0f, 0.0f);
	const glm::vec3 Transform3DComponent::RIGHT(1.0f, 0.0f, 0.0f);


	Transform3DComponent::Transform3DComponent()
		: position(0, 0, 0), scale(1.0f, 1.0f, 1.0f), rotation()
	{
		m_dirty = false;
	}

	Transform3DComponent::~Transform3DComponent(){
	}

	// Position
	const glm::vec3& Transform3DComponent::getPosition() const{
		return position;
	}

	void Transform3DComponent::setPosition(const glm::vec3& p){
		position = p;
		m_dirty = true;
	}

	// Scale
	const glm::vec3& Transform3DComponent::getScale() const{
		return scale;
	}

	void Transform3DComponent::setScale(const glm::vec3& s){
		scale = s;
		m_dirty = true;
	}

	// Rotation
	const glm::quat& Transform3DComponent::getRotation() const{
		return rotation;
	}

	void Transform3DComponent::setRotation(const glm::quat& quat){
		rotation = quat;
		m_dirty = true;
	}

}
