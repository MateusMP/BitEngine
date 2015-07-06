
#include "SceneGraph.h"
#include "GameEngine.h"

namespace BitEngine{

SceneGraph::SceneGraph(){
    root = new SceneNode(NULL);

    // Insert root node
    nodes.push_back(root);
    // No need to insert root on reference maps
}

SceneGraph::~SceneGraph(){
    delete root;
}


void SceneGraph::Update()
{
    root->Update();
}

void SceneGraph::AddObject(SceneNode *parent, SceneNode* node){
    const uint32_t id = nodes.size();
    nodes.push_back(node);
    nodesByID.insert( std::pair<uint32_t, SceneNode*>(id, node) );
    nodesByRef.insert( std::pair<SceneNode*, uint32_t>(node, id) );

    parent->AddChild(node);
}

void SceneGraph::AddObject(SceneNode *node){
    AddObject(root, node);
}

uint32_t SceneGraph::GetObjectID(SceneNode* node)
{
    std::map<SceneNode*, uint32_t>::iterator it = nodesByRef.find( node );
    if (it == nodesByRef.end())
        return -1;
    return it->second;
}

SceneNode* SceneGraph::GetObjectOfID(uint32_t id)
{
    std::map<uint32_t, SceneNode*>::iterator it = nodesByID.find( id );
    if (it == nodesByID.end())
        return nullptr;
    return it->second;
}

void SceneGraph::RemoveObject(SceneNode *obj)
{
    std::map<SceneNode*, uint32_t>::iterator it = nodesByRef.find(obj);
    if (it == nodesByRef.end()){
        printf("Trying to remove unknown node.");
        return;
    }

    const uint32_t swith_id = nodes.size()-1;
    const uint32_t removed_id = it->second;
    if (removed_id == swith_id){
        nodes.pop_back();
        nodesByRef.erase(it);
        nodesByID.erase(removed_id);
    } else {
        SceneNode *switchObj = *(nodes.end());

        // Repositioning last element over removed element
        nodes[removed_id] = switchObj;

        // Removing references to removed elements
        nodesByID.erase(swith_id);
        nodesByRef.erase(it);

        // Recreating references for moved element
        nodesByRef[switchObj] = removed_id;
        nodesByID[removed_id] = switchObj;

        // Rebuild spatial?
    }
}


}
