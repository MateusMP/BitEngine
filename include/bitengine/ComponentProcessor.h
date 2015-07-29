#pragma once

#include "TypeDefinition.h"
#include "Component.h"
#include "ComponentCollection.h"

namespace BitEngine{

class ComponentProcessor
{
public:
	virtual ~ComponentProcessor() {}

private:

	friend class EntitySystem;

	// 
	virtual bool Init() = 0;

	virtual void FrameStart() = 0;
	virtual void FrameMiddle() = 0;
	virtual void FrameEnd() = 0;
};

class ComponentHolderProcessor : public ComponentProcessor
{
	public:
		virtual ~ComponentHolderProcessor() {}

		virtual Component* getComponent(ComponentHandle component) = 0;

	private:
		friend class EntitySystem;
		virtual ComponentHandle CreateComponent(EntityHandle entity) = 0;
		virtual void DestroyComponent(ComponentHandle component) = 0;

		virtual const std::vector<ComponentHandle>& getComponents() const = 0;
};

}