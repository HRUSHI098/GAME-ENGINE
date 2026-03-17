#pragma once

#include "GE/Core/Events/Event.h"
#include <sstream>

namespace GE {

class WindowCloseEvent : public Event {
public:
    WindowCloseEvent() = default;

    GE_EVENT_CLASS_TYPE(WindowClose)
    GE_EVENT_CLASS_CATEGORY(EventCategory::Application)
};

class WindowResizeEvent : public Event {
public:
    WindowResizeEvent(u32 width, u32 height) : m_Width(width), m_Height(height) {}

    u32 GetWidth()  const { return m_Width; }
    u32 GetHeight() const { return m_Height; }

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "WindowResizeEvent: " << m_Width << "x" << m_Height;
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(WindowResize)
    GE_EVENT_CLASS_CATEGORY(EventCategory::Application)

private:
    u32 m_Width, m_Height;
};

} // namespace GE
