#pragma once

#include "BitEngine/Core/ECS/Camera2DComponent.h"
#include "BitEngine/Core/ECS/ComponentProcessor.h"

#include "BitEngine/Core/ECS/Transform2DProcessor.h"
#include "BitEngine/Core/ECS/EntitySystem.h"

#include "BitEngine/Common/VectorBool.h"

namespace BitEngine {

class BE_API Camera2DProcessor : public ComponentProcessor
{
public: // Methods
    Camera2DProcessor(EntitySystem* es, Transform2DProcessor* t2dp);

    /// Processor
    bool Init() override;
    void Stop() override;
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