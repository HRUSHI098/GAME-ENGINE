#pragma once

#include <string>
#include <functional>
#include <sstream>

namespace GE {

// ─── Event categories (bitmask) ──────────────────────────────────────────────
enum class EventCategory : int {
    None        = 0,
    Application = 1 << 0,
    Input       = 1 << 1,
    Keyboard    = 1 << 2,
    Mouse       = 1 << 3,
    MouseButton = 1 << 4,
};

inline EventCategory operator|(EventCategory a, EventCategory b) {
    return static_cast<EventCategory>(static_cast<int>(a) | static_cast<int>(b));
}
inline bool operator&(EventCategory a, EventCategory b) {
    return static_cast<int>(a) & static_cast<int>(b);
}

// ─── Event types ─────────────────────────────────────────────────────────────
enum class EventType {
    None = 0,
    // Window
    WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
    // App
    AppTick, AppUpdate, AppRender,
    // Keyboard
    KeyPressed, KeyReleased, KeyTyped,
    // Mouse
    MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled,
};

// ─── Base Event ──────────────────────────────────────────────────────────────
class Event {
public:
    bool Handled = false;

    virtual EventType     GetEventType()     const = 0;
    virtual EventCategory GetCategoryFlags() const = 0;
    virtual const char*   GetName()          const = 0;
    virtual std::string   ToString()         const { return GetName(); }

    bool IsInCategory(EventCategory cat) const {
        return GetCategoryFlags() & cat;
    }

    virtual ~Event() = default;
};

// ─── Event dispatcher ────────────────────────────────────────────────────────
// Usage:
//   EventDispatcher d(event);
//   d.Dispatch<WindowCloseEvent>([](WindowCloseEvent& e) { ... return true; });
class EventDispatcher {
public:
    explicit EventDispatcher(Event& event) : m_Event(event) {}

    template<typename T, typename F>
    bool Dispatch(const F& func) {
        if (m_Event.GetEventType() == T::GetStaticType()) {
            m_Event.Handled |= func(static_cast<T&>(m_Event));
            return true;
        }
        return false;
    }

private:
    Event& m_Event;
};

inline std::ostream& operator<<(std::ostream& os, const Event& e) {
    return os << e.ToString();
}

} // namespace GE

// Helper macro to reduce boilerplate in event subclasses
#define GE_EVENT_CLASS_TYPE(type)                                              \
    static GE::EventType GetStaticType() { return GE::EventType::type; }      \
    GE::EventType GetEventType() const override { return GetStaticType(); }    \
    const char* GetName() const override { return #type; }

#define GE_EVENT_CLASS_CATEGORY(cat)                                           \
    GE::EventCategory GetCategoryFlags() const override { return cat; }
