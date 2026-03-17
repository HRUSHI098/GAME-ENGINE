#pragma once

#include "GE/Core/Window.h"
#include <SDL2/SDL.h>

namespace GE {

class SDL2Window : public Window {
public:
    explicit SDL2Window(const WindowProps& props);
    ~SDL2Window() override;

    void OnUpdate() override;

    u32 GetWidth()  const override { return m_Data.Width; }
    u32 GetHeight() const override { return m_Data.Height; }

    void SetEventCallback(const EventCallbackFn& cb) override { m_Data.EventCallback = cb; }
    void SetVSync(bool enabled) override;
    bool IsVSync() const override { return m_Data.VSync; }

    void* GetNativeWindow() const override { return m_Window; }

private:
    void Init(const WindowProps& props);
    void Shutdown();

    SDL_Window*   m_Window   = nullptr;
    SDL_GLContext m_Context  = nullptr;   // reserved for future OpenGL use

    struct WindowData {
        std::string    Title;
        u32            Width  = 0;
        u32            Height = 0;
        bool           VSync  = true;
        EventCallbackFn EventCallback;
    };

    WindowData m_Data;
};

} // namespace GE
