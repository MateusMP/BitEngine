#pragma once

#include <list>
#include <string>

#include "Transform.h"

namespace BitEngine{


class SceneNode
{
    public:
        typedef std::list<SceneNode*> NodeList;

        SceneNode(SceneNode* parent);

        // destructor - calls destroy
        virtual ~SceneNode() { Destroy(); }

        // release this object from memory
        void Release() { delete this; }

        // loop through the list and update the children
        virtual void Update();

        // destroy all childrens
        void Destroy();

        // add a child to our custody
        SceneNode* GetParent(void) const;
        void SetParent(SceneNode* NewParent);

        NodeList& GetChilds();

        void AddChild(SceneNode* ChildNode);
        void RemoveChild(SceneNode* ChildNode);

        const std::string& GetName() const;
        void SetName(const std::string& name);

    protected:
        Transform localTranform;
        Transform worldTranform;

    private:
        void UpdateWorldTransform();

        SceneNode* m_parent;
        NodeList m_children;

        std::string m_name;
};



}


