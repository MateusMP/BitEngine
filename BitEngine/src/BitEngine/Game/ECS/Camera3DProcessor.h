#pragma once

#include <unordered_set>

#include "BitEngine/Game/ECS/Camera3DComponent.h"
#include "BitEngine/Game/ECS/ComponentProcessor.h"

#include "BitEngine/Game/ECS/Transform3DProcessor.h"
#include "BitEngine/Game/ECS/EntitySystem.h"

namespace BitEngine {

class BE_API Camera3DProcessor : public ComponentProcessor {
public:
    Camera3DProcessor(EntitySystem* es, Transform3DProcessor* t3dp);

    /// Processor
    void Process();

private:
    //
    static void recalculateViewMatrix(Camera3DComponent& c, const glm::mat4& t);

private:
    Transform3DProcessor* transform3DProcessor;
};
}
