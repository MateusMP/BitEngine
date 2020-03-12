#pragma once

#include "BitEngine/Core/Math.h"
#include "BitEngine/Game/ECS/Component.h"

namespace BitEngine {

class BE_API Camera2DComponent
    : public Component<Camera2DComponent>
{
public:
    Camera2DComponent();

    // Set camera focus position
    // This position will be at the center of the camera bounding view
    // To zoom on the center of the screen, use (screen_width/2, screen_height/2)
    void setLookAt(const glm::vec3& pos);
    glm::vec3 getLookAt() const;

    // The view Matrix
    // Calculated by the Camera2DProcessor
    const glm::mat4& getMatrix() const;

    // Usually the screen resolution
    void setView(int width, int height);

    void setZoom(float z);
    float getZoom() const;

    /**
     *
     * @return vec4( bottomLeftX, bottomLeftY, topRightX, topRightY )
     */
    glm::vec4 getWorldViewArea() const;

    const glm::vec2 screenToWorldCoordinates(const glm::vec2& screenCoord) const;

    // members
    int m_width;
    int m_height;

    glm::vec3 m_lookAt;
    float m_zoom;

    glm::mat4 m_cameraMatrix;
    glm::mat4 m_orthoMatrix;

    bool changed;

};


}
