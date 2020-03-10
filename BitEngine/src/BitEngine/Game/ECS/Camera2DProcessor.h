#pragma once

#include "BitEngine/Game/ECS/Camera2DComponent.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"

#include "BitEngine/Game/ECS/Transform2DProcessor.h"
#include "BitEngine/Game/ECS/EntitySystem.h"

#include "BitEngine/Common/VectorBool.h"

namespace BitEngine {

class BE_API Camera2DProcessor : public ComponentProcessor
{
public: // Methods
    Camera2DProcessor(EntitySystem* es, Transform2DProcessor* t2dp);

    /// Processor
    void Process();

private: // Methods

    /// Processor
    static void recalculateMatrix(Camera2DComponent& c);

private: // Member variables

    /// Component processor

    // Hold the ComponentType for all components this processor cares
    ComponentType Transform2DType;
    ComponentType Camera2DType;

    // Hold the ComponentHolders for all ComponentTypes cared by this processor
    ComponentHolder<Camera2DComponent>* holderCamera;
    ComponentHolder<Transform2DComponent>* holderTransform;
    Transform2DProcessor* transform2DProcessor;
};

}