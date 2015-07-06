#pragma once

#include <map>
#include <vector>

#include "SceneNode.h"

namespace BitEngine{

class SceneGraph
{
    public:
        SceneGraph();

        ~SceneGraph();

        void Update();

        /**
         * Insert a new node as child for the parent
         * @param parent The parent node (Should already exist on the graph)
         * @param node The new inserted node
         */
        void AddObject(SceneNode *parent, SceneNode* node);

        /** Insert a new node under ROOT.
         * @param node Object to be inserted as child of the ROOT element
         */
        void AddObject(SceneNode *node);

        uint32_t GetObjectID(SceneNode* node);

        SceneNode* GetObjectOfID(uint32_t id);

        /**
         * Remove an node from the scene graph
         * @param obj Object to be removed
         */
        void RemoveObject(SceneNode *obj);

    private:
        SceneNode* root;

        // Linear reference
        std::vector<SceneNode*> nodes;

        // Reference to ID
        std::map<SceneNode*, uint32_t> nodesByRef;

        // ID reference
        std::map<uint32_t, SceneNode*> nodesByID;

        // Spatial reference
        // todo
};


}

