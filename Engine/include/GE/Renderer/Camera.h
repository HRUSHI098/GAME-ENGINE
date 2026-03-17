#pragma once

#include "GE/Core/Base.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace GE {

class Camera2D {
public:
    Camera2D(f32 left, f32 right, f32 bottom, f32 top);

    void SetPosition(const glm::vec3& position);
    void SetRotation(f32 rotationDegrees);
    void SetProjection(f32 left, f32 right, f32 bottom, f32 top);

    const glm::vec3& GetPosition()  const { return m_Position; }
    f32              GetRotation()  const { return m_Rotation; }

    const glm::mat4& GetProjectionMatrix()     const { return m_Projection; }
    const glm::mat4& GetViewMatrix()           const { return m_View; }
    const glm::mat4& GetViewProjectionMatrix() const { return m_ViewProjection; }

private:
    void RecalculateViewMatrix();

    glm::mat4 m_Projection;
    glm::mat4 m_View;
    glm::mat4 m_ViewProjection;

    glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    f32       m_Rotation = 0.0f;
};

} // namespace GE
