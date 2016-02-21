#include "Core/ECS/EntitySystem.h"
#include "Core/ECS/Transform2DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform2DProcessor::Transform2DProcessor()
	{
	}


	Transform2DProcessor::~Transform2DProcessor()
	{
	}

	bool Transform2DProcessor::Init()
	{
		getMessenger()->RegisterListener<MsgComponentCreated<Transform2DComponent> >
			(this, BE_MESSAGE_HANDLER(Transform2DProcessor::onTransform2DComponentCreated));
		getMessenger()->RegisterListener<MsgComponentDestroyed<Transform2DComponent> >
			(this, BE_MESSAGE_HANDLER(Transform2DProcessor::onTransform2DComponentDestroyed));

		return true;
	}

	void Transform2DProcessor::Stop()
	{
	}


	void Transform2DProcessor::CalculateLocalModelMatrix(const Transform2DComponent& comp, glm::mat3& mat)
	{
		// T R S
		float cosx = cos(comp.rotation);
		float sinx = sin(comp.rotation);
		mat = glm::transpose(glm::mat3(  comp.scale.x*cosx, comp.scale.y*sinx, comp.position.x,
										-comp.scale.x*sinx, comp.scale.y*cosx, comp.position.y,
										 0.0f,				 0.0f,				 1.0f));
	}

	void Transform2DProcessor::onTransform2DComponentCreated(const BaseMessage& msg_)
	{
		const MsgComponentCreated<Transform2DComponent>& msg = static_cast<const MsgComponentCreated<Transform2DComponent>&>(msg_);
		const uint32 nComponents = msg.component;

		if (localTransform.size() <= nComponents)
		{
			localTransform.resize(nComponents + 1);
			globalTransform.resize(nComponents + 1);
			hierarchy.resize(nComponents + 1);
		}
	}

	void Transform2DProcessor::onTransform2DComponentDestroyed(const BaseMessage& msg_)
	{
		const MsgComponentDestroyed<Transform2DComponent>& msg = static_cast<const MsgComponentDestroyed<Transform2DComponent>&>(msg_);

		// Childs lost their parent. Let them to the previous parent root.
		for (ComponentHandle c : hierarchy[msg.component].childs)
		{
			setParentOf(c, hierarchy[msg.component].parent);
		}
	}

	void Transform2DProcessor::setParentOf(ComponentHandle a, ComponentHandle parent)
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

	void Transform2DProcessor::Process()
	{
		getES()->forAll<Transform2DProcessor, Transform2DComponent>(*this,
			[](Transform2DProcessor& self, ComponentHandle c, Transform2DComponent& transform)
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
		getES()->forAll<Transform2DProcessor, Transform2DComponent>(*this,
			[](Transform2DProcessor& self, ComponentHandle c, Transform2DComponent& transform)
			{
				self.recalcGlobalTransform(c, self.hierarchy[c]);
			}
		);
	}

	void Transform2DProcessor::recalcGlobalTransform(ComponentHandle handle, Hierarchy &t)
	{
		if (t.dirty)
		{
			if (t.parent != 0) {
				globalTransform[handle] = globalTransform[t.parent] * localTransform[handle];
			} else {
				globalTransform[handle] = localTransform[handle];
			}

			t.dirty = false;

			for (const ComponentHandle c : t.childs)
            {
				hierarchy[c].dirty = true;
				recalcGlobalTransform(c, hierarchy[c]);
			}
		}
	}
}
