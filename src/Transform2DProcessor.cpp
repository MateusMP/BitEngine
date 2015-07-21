#include "Transform2DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform2DProcessor::Transform2DProcessor()
	{
	}


	Transform2DProcessor::~Transform2DProcessor()
	{
	}

	bool Transform2DProcessor::Init() {
		return true;
	}

	int Transform2DProcessor::calculateParentRootDistance(Transform2DComponent* t){
		
		if (t->parent == nullptr)
			return 0;

		// TODO: Remove recursion
		int k = 0;
		t = t->parent;
		if (t->m_dirty){
			int x = calculateParentRootDistance(t);
			t->m_nParents = x;
			t->m_dirty = Transform2DComponent::DIRTY_DATA;
			k = x + 1;
		}
		else {
			k = t->m_nParents + 1;
		}		

		return k;
	}

	void Transform2DProcessor::FrameEnd() {

		// Recalculate distance to root
		for (Transform2DComponent* t : components.getValidComponentsRef())
		{
			// printf("before Verify T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);
			if (t->m_dirty & Transform2DComponent::DIRTY_PARENT){
				t->m_nParents = calculateParentRootDistance(t);
				t->m_dirty = Transform2DComponent::DIRTY_DATA;
			}
			// printf("after Verify T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);
		}
		
		// Copy vector
		std::vector< Transform2DComponent* > ordered = components.getValidComponentsRef();

		// sort
		std::sort(ordered.begin(), ordered.end(), 
					[](Transform2DComponent* a, Transform2DComponent* b){
							return a->m_nParents < b->m_nParents;
					} );

		// Update transform matrix starting from the parents
		for (Transform2DComponent* t : ordered){
			// printf("Ordered T2C: %p parent: %p, nParents: %d\n", t, t->parent, t->m_nParents);

			if (t->m_dirty & Transform2DComponent::DIRTY_DATA){
				t->recalculateMatrix();
			}
		}

	}

	ComponentHandle Transform2DProcessor::CreateComponent(EntityHandle entity) {
		return components.newComponent();
	}

	void Transform2DProcessor::DestroyComponent(ComponentHandle component) {
		components.removeComponent(component);
	}

	Component* Transform2DProcessor::getComponent(ComponentHandle component) {
		return components.getComponent(component);
	}

	std::vector<ComponentHandle>& Transform2DProcessor::getComponents() {
		return components.getValidComponents();
	}

}