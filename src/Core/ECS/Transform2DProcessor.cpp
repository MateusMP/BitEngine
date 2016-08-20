#include "Core/ECS/EntitySystem.h"
#include "Core/ECS/Transform2DProcessor.h"

#include <algorithm>

namespace BitEngine{

	Transform2DProcessor::Transform2DProcessor(Messenger* m)
		: ComponentProcessor(m)
	{
	}


	Transform2DProcessor::~Transform2DProcessor()
	{
	}

	bool Transform2DProcessor::Init()
	{
		getMessenger()->registerListener<MsgComponentCreated<Transform2DComponent> >(this);
		getMessenger()->registerListener<MsgComponentDestroyed<Transform2DComponent> >(this);

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

	void Transform2DProcessor::onMessage(const MsgComponentCreated<Transform2DComponent>& msg)
	{
		const u32 nComponents = msg.component;

		if (hierarchy.size() <= nComponents)
		{
			hierarchy.resize(nComponents + 1);
		}
	}

	void Transform2DProcessor::onMessage(const MsgComponentDestroyed<Transform2DComponent>& msg)
	{
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
		getES()->forEach<Transform2DComponent, SceneTransform2DComponent>(
			[this] (ComponentRef<Transform2DComponent> transform, ComponentRef<SceneTransform2DComponent> scene)
			{
				if (transform->m_dirty)
				{
					transform->m_dirty = false;
					CalculateLocalModelMatrix(transform.ref(), scene->m_local);
					hierarchy[scene.getComponentID()].dirty = true;
				}
			});

		ComponentHolder<SceneTransform2DComponent>* holder = getES()->getHolder<SceneTransform2DComponent>();

		// Update globalTransform of valid components
		getES()->forEach<Transform2DComponent, SceneTransform2DComponent>(
			[this, holder](ComponentRef<Transform2DComponent> transform, ComponentRef<SceneTransform2DComponent> scene)
			{
				const ComponentHandle c = scene.getComponentID();
				recalcGlobalTransform(holder, scene.ref(), hierarchy[c]);
			}
		);
	}

	void Transform2DProcessor::recalcGlobalTransform(ComponentHolder<SceneTransform2DComponent>* holder, SceneTransform2DComponent& scene, Hierarchy &t)
	{
		if (t.dirty)
		{
			if (t.parent != 0) {
				scene.m_global = holder->getComponent(t.parent)->m_global *scene.m_local;
			} else {
				scene.m_global = scene.m_local;
			}

			t.dirty = false;

			for (const ComponentHandle c : t.childs)
            {
				hierarchy[c].dirty = true;
				recalcGlobalTransform(holder, *(holder->getComponent(c)), hierarchy[c]);
			}
		}
	}
}
