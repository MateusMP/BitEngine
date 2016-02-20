#include "Core/ECS/Transform3DProcessor.h"
#include "Core/ECS/EntitySystem.h"
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

	void Transform3DProcessor::Stop() {
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
		//for (ComponentHandle c : components.getValidComponents())
		getES()->forAll<Transform3DComponent, Transform3DProcessor*>(
			[](ComponentHandle c, Transform3DComponent& transform, Transform3DProcessor* self)
			{
				Transform3DComponent* t = &transform;
				if ( t->m_dirty ) 
				{
					t->m_dirty = false;
					CalculateLocalModelMatrix(t, self->localTransform[c]);
					self->hierarchy[c].dirty = true;
				}
			}, this
		);

		// Update globalTransform of valid components
		getES()->forAll<Transform3DComponent, Transform3DProcessor*>(
			[](ComponentHandle c, Transform3DComponent& transform, Transform3DProcessor* self)
			{
				self->RecalcGlobal(self->hierarchy[c]);
			}, this
		);
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

}