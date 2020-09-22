#pragma once

#include "BitEngine/Game/ECS/Camera2DComponent.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"

#include "BitEngine/Game/ECS/Transform2DProcessor.h"
#include "BitEngine/Game/ECS/EntitySystem.h"

#include "BitEngine/Common/VectorBool.h"

namespace BitEngine {

class BE_API Camera2DProcessor : public ComponentProcessor {
public: // Methods
    Camera2DProcessor(EntitySystem* es);

    /// Processor
    void Process();

private: // Methods
    /// Processor
    static void recalculateMatrix(Camera2DComponent& c);

private:
};
}