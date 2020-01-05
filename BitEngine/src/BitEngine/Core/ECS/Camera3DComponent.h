#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "BitEngine/Core/ECS/Component.h"

namespace BitEngine {

class BE_API Camera3DComponent : public Component<Camera3DComponent>
{
public:

    enum ProjectionMode {
        ORTHOGONAL = 1,
        PERSPECTIVE,

    };

    Camera3DComponent()
        : m_projectionType(PERSPECTIVE),
        m_width(1280), m_height(720),
        m_fov(45.0f), m_aspect(16.0f / 9.0f), m_near(1.0f), m_far(1000.0f),
        m_lookAt(0.0f, 0.0f, 0.0f), m_up(0.0f, 1.0f, 0.0f)
    {
        recalculateProjection();
    }

    ~Camera3DComponent() {}

    void setView(int width, int height) { m_width = width; m_height = height; }

    void setFOV(float fov) { m_fov = fov; }
    void setNearFar(float n, float f) { m_near = n; m_far = f; }
    void setLookAt(const glm::vec3& lookAt) { m_lookAt = lookAt; }
    void setUp(const glm::vec3& up) { m_up = up; }

    const glm::mat4& getProjection() const { return m_projection; }
    const glm::mat4& getView() const { return m_viewMatrix; }

private:
    friend class Camera3DProcessor;

    void recalculateProjection();

    ProjectionMode m_projectionType;
    int m_width;
    int m_height;
    float m_fov;
    float m_aspect;
    float m_near;
    float m_far;

    glm::vec3 m_lookAt;
    glm::vec3 m_up;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projection;
};


}
