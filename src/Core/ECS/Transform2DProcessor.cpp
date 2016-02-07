#include "Core/ECS/Transform2DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform2DProcessor::Transform2DProcessor()
	{
	}


	Transform2DProcessor::~Transform2DProcessor()
	{
	}

	bool Transform2DProcessor::Init(BaseEntitySystem* es) {
		RegisterListener(this);

		return true;
	}

	void Transform2DProcessor::Stop()
	{
		UnregisterListener(this);
	}


	void Transform2DProcessor::CalculateLocalModelMatrix(const Transform2DComponent* comp, glm::mat3& mat)
	{
		// T R S
		float cosx = cos(comp->rotation);
		float sinx = sin(comp->rotation);
		mat = glm::transpose(glm::mat3(  comp->scale.x*cosx, comp->scale.y*sinx, comp->position.x,
										-comp->scale.x*sinx, comp->scale.y*cosx, comp->position.y,
										 0.0f,				 0.0f,				 1.0f));
	}

	void Transform2DProcessor::Process()
	{
		// Recalculate localTransform
		for (ComponentHandle c : components.getValidComponents())
		{
			Transform2DComponent* t = (Transform2DComponent*)getComponent(c);
			if (t->m_dirty)
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

	void Transform2DProcessor::RecalcGlobal(Hierarchy &t)
	{
		if (t.dirty)
		{
			if (t.parent != 0) {
				globalTransform[t.self] = globalTransform[t.parent] * localTransform[t.self];
			} else {
				globalTransform[t.self] = localTransform[t.self];
			}

			t.dirty = false;

			for (const ComponentHandle c : t.childs)
            {
				hierarchy[c].dirty = true;
				RecalcGlobal(hierarchy[c]);
			}
		}
	}

	ComponentHandle Transform2DProcessor::AllocComponent() {
		return components.newComponent();
	}

	void Transform2DProcessor::DeallocComponent(ComponentHandle component) {
		components.removeComponent(component);
	}

	Component* Transform2DProcessor::getComponent(ComponentHandle component) {
		return components.getComponent(component);
	}

	const std::vector<ComponentHandle>& Transform2DProcessor::getComponents() const {
		return components.getValidComponents();
	}
}
