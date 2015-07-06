#include "SceneNode.h"

namespace BitEngine{


SceneNode::SceneNode(SceneNode* parent)
    : m_parent(parent)
{
    m_name = "unamed";
}

void SceneNode::UpdateWorldTransform()
{
    // TODO: Problema com 3 nos
    // no do meio pode estar "limpo" e nao receber mudanca do pai

    // I need update if my parent changed
    if (worldTranform.IsDirty() || localTranform.IsDirty() || m_parent->localTranform.IsDirty())
    {
        // Update parent first
        m_parent->UpdateWorldTransform();

        worldTranform = m_parent->worldTranform * localTranform;
        worldTranform.SetClean();
    }
}

void SceneNode::Update()
{
    for( NodeList::iterator i = m_children.begin();  i != m_children.end(); i++ )
    {
        (*i)->Update();
    }
}

// destroy all the children
void SceneNode::Destroy()
{
    for( NodeList::iterator i = m_children.begin(); i != m_children.end(); i++ )
    {
      (*i)->Release();
    }

    m_children.clear();
}

SceneNode::NodeList& SceneNode::GetChilds()
{
    return m_children;
}

SceneNode* SceneNode::GetParent(void) const
{
    return(m_parent);
};

void SceneNode::SetParent(SceneNode* NewParent)
{
    if(NULL != m_parent)
    {
        m_parent->RemoveChild(this);
    }
    m_parent = NewParent;
};

void SceneNode::AddChild(SceneNode* child)
{
    if (child == nullptr)
        return;

    if (child->GetParent() != nullptr)
    {
        child->SetParent(this);
    }

    m_children.push_back(child);
};

void SceneNode::RemoveChild(SceneNode* child)
{
    if (child == nullptr || m_children.empty())
        return;

    for (NodeList::iterator it = m_children.begin(); it != m_children.end(); ++it)
    {
        if (*it == child)
        {
            m_children.erase(it);
            break;
        }
    }
};

const std::string& SceneNode::GetName() const
{
    return m_name;
}

void SceneNode::SetName(const std::string& name)
{
    m_name = name;
}


}
