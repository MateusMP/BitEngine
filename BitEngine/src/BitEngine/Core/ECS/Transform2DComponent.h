#pragma once

#include "BitEngine/Core/Math.h"
#include "BitEngine/Core/ECS/Component.h"

namespace BitEngine {

class BE_API Transform2DComponent :
    public Component<Transform2DComponent>
{
public:
    Transform2DComponent();
    ~Transform2DComponent();

    // Position
    const glm::vec2& getLocalPosition() const; // returns LOCAL position
    template<typename T>
    void setLocalPosition(T x, T y) // sets LOCAL position
    {
        position.x = (float)x;
        position.y = (float)y;
        m_dirty = true;
    }
    void setLocalPosition(const glm::vec2& p); // sets LOCAL position

    // Scale
    const glm::vec2& getLocalScale() const; // get LOCAL scale
    void setLocalScale(const glm::vec2& s); // sets LOCAL scale

    // Rotation
    float getLocalRotation() const; // get LOCAL rotation
    void setLocalRotation(float rad); // sets LOCAL rotation

    /* // TODO ADD THIS AGAIN SOMEWHERE
    float getWorldAngle() const {
        return atan2(m_modelMatrix[0][1], m_modelMatrix[1][1]);
    }

    glm::vec2 getRight() const {
        return glm::mat2(m_modelMatrix) * glm::vec2(1, 0);
    }
    */

    glm::vec2 position;
    glm::vec2 scale;
    float rotation;

    bool m_dirty;
};

class BE_API SceneTransform2DComponent
    : public Component<SceneTransform2DComponent>
{
public:
    const ComponentHandle getParent() const {
        return m_parent;
    }

    void setParent(ComponentHandle p) {
        m_parent = p;
    }

    const glm::mat3& getLocal() const {
        return m_local;
    }

    glm::mat3& getLocal() {
        return m_local;
    }

    const glm::mat3& getGlobal() const {
        return m_global;
    }

    void setGlobal(const glm::mat3& m) {
        m_global = m;
    }

    ComponentHandle m_parent;
    glm::mat3 m_local;
    glm::mat3 m_global;
};
}
