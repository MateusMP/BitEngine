#include "Transform3DComponent.h"

namespace BitEngine{

	Transform3DComponent::Transform3DComponent()
		: m_dirty(DIRTY_ALL), m_nParents(0), parent(0), position(0, 0, 0), scale(1.0f, 1.0f, 1.0f), rotation()
	{
	}

	Transform3DComponent::~Transform3DComponent(){
	}

	// Position
	const glm::vec3& Transform3DComponent::getPosition() const{
		return position;
	}

	void Transform3DComponent::setPosition(const glm::vec3& p){
		position = p;
	}

	// Scale
	const glm::vec3& Transform3DComponent::getScale() const{
		return scale;
	}

	void Transform3DComponent::setScale(const glm::vec3& s){
		scale = s;
	}

	// Rotation
	const glm::quat& Transform3DComponent::getRotation() const{
		return rotation;
	}

	void Transform3DComponent::setRotation(const glm::quat& quat){
		rotation = quat;
	}

	/**
	* Retrieves the last calculated model matrix
	* This is updated by Transform2DProcessor
	*/
	const glm::mat4& Transform3DComponent::getMatrix() const{
		return m_modelMatrix;
	}

	void Transform3DComponent::setParent(ComponentHandle p){
		parent = p;
	}

	ComponentHandle Transform3DComponent::getParent() const{
		return parent;
	}



}