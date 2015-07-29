#include "Transform3DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform3DProcessor::Transform3DProcessor()
	{
	}


	Transform3DProcessor::~Transform3DProcessor()
	{
	}

	bool Transform3DProcessor::Init() {
		return true;
	}

	void Transform3DProcessor::calculateModelMatrix(Transform3DComponent* comp, glm::mat4& mat)
	{
		// T R S
		// mat = glm::translate(glm::mat4(1), comp->position) * glm::mat4_cast(comp->rotation) * glm::scale(glm::mat4(1), comp->scale);
		mat = glm::translate(glm::mat4_cast(comp->rotation) * glm::scale(glm::mat4(1), comp->scale), comp->position);

		if (comp->parent)
		{
			Transform3DComponent* parentT = components.getComponent(comp->parent);
			mat = parentT->getMatrix() * mat;
		}
	}

	int Transform3DProcessor::calculateParentRootDistance(Transform3DComponent* t){

		if (t->parent == 0)
			return 0;

		// TODO: Remove recursion
		int k = 0;
		t = components.getComponent(t->parent);
		if (t->m_dirty){
			int x = calculateParentRootDistance(t);
			t->m_nParents = x;
			t->m_dirty = Transform3DComponent::DIRTY_DATA;
			k = x + 1;
		}
		else {
			k = t->m_nParents + 1;
		}

		return k;
	}

	void Transform3DProcessor::FrameEnd() {

		// Recalculate distance to root
		for (Transform3DComponent* t : components.getValidComponentsRef())
		{
			// printf("before Verify T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);
			if (t->m_dirty & Transform3DComponent::DIRTY_PARENT){
				t->m_nParents = calculateParentRootDistance(t);
				t->m_dirty = Transform3DComponent::DIRTY_DATA;
			}
			// printf("after Verify T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);
		}

		// Copy vector
		std::vector< Transform3DComponent* > ordered = components.getValidComponentsRef();

		// sort
		std::sort(ordered.begin(), ordered.end(),
			[](Transform3DComponent* a, Transform3DComponent* b){
			return a->m_nParents < b->m_nParents;
		});

		// Update transform matrix starting from the parents
		for (Transform3DComponent* t : ordered){
			// printf("Ordered T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);

			if (t->m_dirty & Transform3DComponent::DIRTY_DATA){
				calculateModelMatrix(t, t->m_modelMatrix);
			}
		}

	}

	ComponentHandle Transform3DProcessor::CreateComponent(EntityHandle entity) {
		return components.newComponent();
	}

	void Transform3DProcessor::DestroyComponent(ComponentHandle component) {
		components.removeComponent(component);
	}

	Component* Transform3DProcessor::getComponent(ComponentHandle component) {
		return components.getComponent(component);
	}

	const std::vector<ComponentHandle>& Transform3DProcessor::getComponents() const {
		return components.getValidComponents();
	}

}