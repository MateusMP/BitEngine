#include "bitengine/Core/ECS/Transform3DProcessor.h"
#include "bitengine/Core/ECS/EntitySystem.h"
#include <algorithm>

namespace BitEngine{

	Transform3DProcessor::Transform3DProcessor(Messenger* m)
		: ComponentProcessor(m)
	{
	}

	Transform3DProcessor::~Transform3DProcessor()
	{
	}

	bool Transform3DProcessor::Init()
	{
		subscribe<MsgComponentCreated<Transform3DComponent> >(&Transform3DProcessor::onMessage, this);
		subscribe<MsgComponentDestroyed<Transform3DComponent> >(&Transform3DProcessor::onMessage, this);
		return true;
	}

	void Transform3DProcessor::Stop() {
	}

	void Transform3DProcessor::CalculateLocalModelMatrix(const Transform3DComponent& comp, glm::mat4& mat)
	{
		// T R S
		// mat = glm::translate(glm::mat4(1), comp->position) * glm::mat4_cast(comp->rotation) * glm::scale(glm::mat4(1), comp->scale);
		mat = glm::translate(glm::mat4_cast(comp.rotation) * glm::scale(glm::mat4(1), comp.scale), comp.position);
	}

	void Transform3DProcessor::onMessage(const MsgComponentCreated<Transform3DComponent>& msg)
	{
		const u32 nComponents = msg.component;

		if (localTransform.size() <= nComponents)
		{
			localTransform.resize(nComponents + 1);
			globalTransform.resize(nComponents + 1);
			hierarchy.resize(nComponents + 1);
		}
	}

	void Transform3DProcessor::onMessage(const MsgComponentDestroyed<Transform3DComponent>& msg)
	{
		// Childs lost their parent. Let them to the previous parent root.
		for (ComponentHandle c : hierarchy[msg.component].childs)
		{
			setParentOf(c, hierarchy[msg.component].parent);
		}
	}

	void Transform3DProcessor::Process()
	{
		/*
		// Recalculate localTransform
		//for (ComponentHandle c : components.getValidComponents())
		getES()->forAll<Transform3DProcessor, Transform3DComponent>(*this,
			[](Transform3DProcessor& self, ComponentHandle c, Transform3DComponent& transform)
			{
				if (transform.m_dirty)
				{
					transform.m_dirty = false;
					CalculateLocalModelMatrix(transform, self.localTransform[c]);
					self.hierarchy[c].dirty = true;
				}
			}
		);

		// Update globalTransform of valid components
		getES()->forAll<Transform3DProcessor, Transform3DComponent>(*this,
			[](Transform3DProcessor& self, ComponentHandle c, Transform3DComponent& transform)
			{
				self.recalcGlobalTransform(c, self.hierarchy[c]);
			}
		);
		*/
	}

	void Transform3DProcessor::recalcGlobalTransform(ComponentHandle handle, Hierarchy &t)
	{
		if (t.dirty)
		{
			if (t.parent != 0)
				globalTransform[handle] = globalTransform[t.parent] * localTransform[handle];
			else
				globalTransform[handle] = localTransform[handle];

			t.dirty = false;

			for (const ComponentHandle c : t.childs) {
				hierarchy[c].dirty = true;
				recalcGlobalTransform(c, hierarchy[c]);
			}
		}
	}

	void Transform3DProcessor::setParentOf(ComponentHandle a, ComponentHandle parent)
    {
        const ComponentHandle prevParent = hierarchy[a].parent;
        if (prevParent == parent)
            return;

        // Remove child from previous parent
        hierarchy[prevParent].removeChild(a);

        // Set parent for given component
        hierarchy[a].parent = parent;
        hierarchy[a].dirty = true;

        // Add child for new parent
        hierarchy[parent].addChild(a);
    }

}
