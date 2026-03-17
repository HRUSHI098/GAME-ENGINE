#include "GE/Renderer/Camera.h"

namespace GE {

Camera2D::Camera2D(f32 left, f32 right, f32 bottom, f32 top) {
    m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_View       = glm::mat4(1.0f);
    m_ViewProjection = m_Projection * m_View;
}

void Camera2D::SetPosition(const glm::vec3& position) {
    m_Position = position;
    RecalculateViewMatrix();
}

void Camera2D::SetRotation(f32 rotationDegrees) {
    m_Rotation = rotationDegrees;
    RecalculateViewMatrix();
}

void Camera2D::SetProjection(f32 left, f32 right, f32 bottom, f32 top) {
    m_Projection = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    m_ViewProjection = m_Projection * m_View;
}

void Camera2D::RecalculateViewMatrix() {
    glm::mat4 transform = glm::translate(glm::mat4(1.0f), m_Position)
        * glm::rotate(glm::mat4(1.0f), glm::radians(m_Rotation), glm::vec3(0, 0, 1));

    m_View = glm::inverse(transform);
    m_ViewProjection = m_Projection * m_View;
}

} // namespace GE
