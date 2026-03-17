#pragma once

#include "GE/Core/Events/Event.h"
#include <sstream>

namespace GE {

enum class MouseButton : int {
    Left   = 1,
    Middle = 2,
    Right  = 3,
    X1     = 4,
    X2     = 5,
};

class MouseMovedEvent : public Event {
public:
    MouseMovedEvent(f32 x, f32 y) : m_X(x), m_Y(y) {}

    f32 GetX() const { return m_X; }
    f32 GetY() const { return m_Y; }

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "MouseMovedEvent: " << m_X << ", " << m_Y;
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(MouseMoved)
    GE_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

private:
    f32 m_X, m_Y;
};

class MouseScrolledEvent : public Event {
public:
    MouseScrolledEvent(f32 xOffset, f32 yOffset)
        : m_XOffset(xOffset), m_YOffset(yOffset) {}

    f32 GetXOffset() const { return m_XOffset; }
    f32 GetYOffset() const { return m_YOffset; }

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "MouseScrolledEvent: " << m_XOffset << ", " << m_YOffset;
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(MouseScrolled)
    GE_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::Input)

private:
    f32 m_XOffset, m_YOffset;
};

class MouseButtonEvent : public Event {
public:
    MouseButton GetMouseButton() const { return m_Button; }
    GE_EVENT_CLASS_CATEGORY(EventCategory::Mouse | EventCategory::MouseButton | EventCategory::Input)

protected:
    explicit MouseButtonEvent(MouseButton button) : m_Button(button) {}
    MouseButton m_Button;
};

class MouseButtonPressedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonPressedEvent(MouseButton button)
        : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "MouseButtonPressedEvent: " << static_cast<int>(m_Button);
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(MouseButtonPressed)
};

class MouseButtonReleasedEvent : public MouseButtonEvent {
public:
    explicit MouseButtonReleasedEvent(MouseButton button)
        : MouseButtonEvent(button) {}

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "MouseButtonReleasedEvent: " << static_cast<int>(m_Button);
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(MouseButtonReleased)
};

} // namespace GE
