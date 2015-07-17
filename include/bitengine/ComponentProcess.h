#pragma once

#include "TypeDefinition.h"
#include "Component.h"
#include "ComponentCollection.h"

namespace BitEngine{

class IComponentProcess
{
private:
	friend class EntitySystem;

	// 
	virtual bool Init() = 0;

	virtual void Process() = 0;

	virtual ComponentHandle CreateComponent() = 0;
	virtual void DestroyComponent(ComponentHandle component) = 0;

	virtual Component* getComponent(ComponentHandle hdl) = 0;
};

template<typename CompClass>
class ComponentHolderProcess : public IComponentProcess
{	
	private:
	virtual void Process() = 0;

	virtual ComponentHandle CreateComponent() = 0;
	virtual void DestroyComponent(ComponentHandle component) = 0;

	virtual Component* getComponent(ComponentHandle hdl) = 0;

	public:
		virtual std::vector<CompClass*>& getComponents() = 0;

};

}