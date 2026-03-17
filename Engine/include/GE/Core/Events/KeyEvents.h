#pragma once

#include "GE/Core/Events/Event.h"
#include <sstream>

namespace GE {

// ─── Key codes (mirrors SDL scancode values for easy translation) ────────────
enum class KeyCode : int {
    // Printable
    Space      = 32,
    A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    // Numbers
    D0 = 48, D1, D2, D3, D4, D5, D6, D7, D8, D9,
    // Function keys
    F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    // Navigation
    Escape = 256, Enter, Tab, Backspace, Insert, Delete,
    Right, Left, Down, Up,
    // Modifiers
    LeftShift = 340, LeftControl, LeftAlt,
    RightShift = 344, RightControl, RightAlt,
};

class KeyEvent : public Event {
public:
    KeyCode GetKeyCode() const { return m_KeyCode; }
    GE_EVENT_CLASS_CATEGORY(EventCategory::Keyboard | EventCategory::Input)

protected:
    explicit KeyEvent(KeyCode keycode) : m_KeyCode(keycode) {}
    KeyCode m_KeyCode;
};

class KeyPressedEvent : public KeyEvent {
public:
    KeyPressedEvent(KeyCode keycode, bool isRepeat = false)
        : KeyEvent(keycode), m_IsRepeat(isRepeat) {}

    bool IsRepeat() const { return m_IsRepeat; }

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "KeyPressedEvent: " << static_cast<int>(m_KeyCode)
           << " (repeat=" << m_IsRepeat << ")";
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(KeyPressed)

private:
    bool m_IsRepeat;
};

class KeyReleasedEvent : public KeyEvent {
public:
    explicit KeyReleasedEvent(KeyCode keycode) : KeyEvent(keycode) {}

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "KeyReleasedEvent: " << static_cast<int>(m_KeyCode);
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(KeyReleased)
};

class KeyTypedEvent : public KeyEvent {
public:
    explicit KeyTypedEvent(KeyCode keycode) : KeyEvent(keycode) {}

    std::string ToString() const override {
        std::ostringstream ss;
        ss << "KeyTypedEvent: " << static_cast<int>(m_KeyCode);
        return ss.str();
    }

    GE_EVENT_CLASS_TYPE(KeyTyped)
};

} // namespace GE
