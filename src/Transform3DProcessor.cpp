#include "Transform3DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform3DProcessor::Transform3DProcessor()
	{
	}

	Transform3DProcessor::~Transform3DProcessor()
	{
	}

	bool Transform3DProcessor::Init(BaseEntitySystem* es) {
		RegisterListener(this);
		return true;
	}

	void Transform3DProcessor::Stop() {
		UnregisterListener(this);
	}

	void Transform3DProcessor::CalculateLocalModelMatrix(const Transform3DComponent* comp, glm::mat4& mat)
	{
		// T R S
		// mat = glm::translate(glm::mat4(1), comp->position) * glm::mat4_cast(comp->rotation) * glm::scale(glm::mat4(1), comp->scale);
		mat = glm::translate(glm::mat4_cast(comp->rotation) * glm::scale(glm::mat4(1), comp->scale), comp->position);
	}

	void Transform3DProcessor::Process() 
	{
		// Recalculate localTransform
		for (ComponentHandle c : components.getValidComponents())
		{
			Transform3DComponent* t = (Transform3DComponent*)getComponent(c);
			if ( t->m_dirty ) 
			{
				t->m_dirty = false;
				CalculateLocalModelMatrix(t, localTransform[c]);
				hierarchy[c].dirty = true;
			} 
		}

		// Update globalTransform of valid components
		for (ComponentHandle c : getComponents())
		{
			RecalcGlobal(hierarchy[c]);
		}
	}

	void Transform3DProcessor::RecalcGlobal(Hierarchy &t)
	{
		if (t.dirty)
		{
			if (t.parent != 0)
				globalTransform[t.self] = globalTransform[t.parent] * localTransform[t.self];
			else
				globalTransform[t.self] = localTransform[t.self];
			
			t.dirty = false;

			for (const ComponentHandle c : t.childs) {
				hierarchy[c].dirty = true;
				RecalcGlobal(hierarchy[c]);
			}
		}
	}

	ComponentHandle Transform3DProcessor::AllocComponent() {
		return components.newComponent();
	}

	void Transform3DProcessor::DeallocComponent(ComponentHandle component) {
		components.removeComponent(component);
	}

	Component* Transform3DProcessor::getComponent(ComponentHandle component) {
		return components.getComponent(component);
	}

	const std::vector<ComponentHandle>& Transform3DProcessor::getComponents() const {
		return components.getValidComponents();
	}

}