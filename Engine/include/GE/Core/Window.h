#pragma once

#include "GE/Core/Base.h"
#include "GE/Core/Events/Event.h"
#include <string>
#include <functional>

namespace GE {

struct WindowProps {
    std::string Title;
    u32 Width;
    u32 Height;

    explicit WindowProps(std::string title  = "Game Engine",
                         u32        width   = 1280,
                         u32        height  = 720)
        : Title(std::move(title)), Width(width), Height(height) {}
};

// Abstract window interface — implemented per platform.
class Window {
public:
    using EventCallbackFn = std::function<void(Event&)>;

    virtual ~Window() = default;

    virtual void OnUpdate() = 0;

    virtual u32 GetWidth()  const = 0;
    virtual u32 GetHeight() const = 0;

    virtual void SetEventCallback(const EventCallbackFn& callback) = 0;
    virtual void SetVSync(bool enabled) = 0;
    virtual bool IsVSync() const = 0;

    // Returns the native window handle (SDL_Window* cast to void*)
    virtual void* GetNativeWindow() const = 0;

    // Factory — creates the platform-appropriate window
    static Scope<Window> Create(const WindowProps& props = WindowProps{});
};

} // namespace GE
