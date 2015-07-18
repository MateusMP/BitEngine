#pragma once

#include "TypeDefinition.h"
#include "Component.h"
#include "ComponentCollection.h"

namespace BitEngine{

class ComponentProcessor
{
private:
	friend class EntitySystem;

	// 
	virtual bool Init() = 0;

	virtual void Process() = 0;
};

class ComponentHolderProcessor : public ComponentProcessor
{
	public:
		virtual Component* getComponent(ComponentHandle component) = 0;

	private:
		friend class EntitySystem;
		virtual ComponentHandle CreateComponent() = 0;
		virtual void DestroyComponent(ComponentHandle component) = 0;

		virtual std::vector<ComponentHandle>& getComponents() = 0;
};

}