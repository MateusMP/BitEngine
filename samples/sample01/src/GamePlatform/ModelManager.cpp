#include "GamePlatform/ModelManager.h"

ModelManager::~ModelManager()
{
    for (auto& it : m_models) {
        ShaderModelHolder& smh = it.second;
        for (auto it2 : smh.modelsByName) {
            delete it2.second;
        }
    }
}

bool ModelManager::Init() {
    return true;
}