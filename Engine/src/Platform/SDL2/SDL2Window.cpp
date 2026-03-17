#include "GE/Platform/SDL2/SDL2Window.h"
#include "GE/Core/Log.h"
#include "GE/Core/Events/WindowEvents.h"
#include "GE/Core/Events/KeyEvents.h"
#include "GE/Core/Events/MouseEvents.h"

namespace GE {

// ─── Factory ─────────────────────────────────────────────────────────────────
Scope<Window> Window::Create(const WindowProps& props) {
    return MakeScope<SDL2Window>(props);
}

// ─── SDL2Window ──────────────────────────────────────────────────────────────
SDL2Window::SDL2Window(const WindowProps& props) {
    Init(props);
}

SDL2Window::~SDL2Window() {
    Shutdown();
}

void SDL2Window::Init(const WindowProps& props) {
    m_Data.Title  = props.Title;
    m_Data.Width  = props.Width;
    m_Data.Height = props.Height;

    GE_CORE_INFO("Creating window '{}' ({}x{})", props.Title, props.Width, props.Height);

    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0) {
        GE_CORE_FATAL("SDL_Init failed: {}", SDL_GetError());
        return;
    }

    m_Window = SDL_CreateWindow(
        props.Title.c_str(),
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        static_cast<int>(props.Width), static_cast<int>(props.Height),
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    if (!m_Window) {
        GE_CORE_FATAL("SDL_CreateWindow failed: {}", SDL_GetError());
    }

    SetVSync(true);
}

void SDL2Window::Shutdown() {
    if (m_Window) {
        SDL_DestroyWindow(m_Window);
        m_Window = nullptr;
    }
}

// ─── Translate SDL keysym to GE KeyCode ──────────────────────────────────────
static KeyCode SDLKeysymToGE(SDL_Keycode sym) {
    // SDL2 keycodes >= 0x40000000 are special keys; map the common ones.
    // Printable ASCII maps directly to our KeyCode enum values.
    if (sym >= 32 && sym <= 122) return static_cast<KeyCode>(sym & ~0x20 | (sym >= 'a' && sym <= 'z' ? 0x20 : 0));
    switch (sym) {
        case SDLK_ESCAPE:    return KeyCode::Escape;
        case SDLK_RETURN:    return KeyCode::Enter;
        case SDLK_TAB:       return KeyCode::Tab;
        case SDLK_BACKSPACE: return KeyCode::Backspace;
        case SDLK_INSERT:    return KeyCode::Insert;
        case SDLK_DELETE:    return KeyCode::Delete;
        case SDLK_RIGHT:     return KeyCode::Right;
        case SDLK_LEFT:      return KeyCode::Left;
        case SDLK_DOWN:      return KeyCode::Down;
        case SDLK_UP:        return KeyCode::Up;
        case SDLK_F1:        return KeyCode::F1;
        case SDLK_F2:        return KeyCode::F2;
        case SDLK_F3:        return KeyCode::F3;
        case SDLK_F4:        return KeyCode::F4;
        case SDLK_F5:        return KeyCode::F5;
        case SDLK_F6:        return KeyCode::F6;
        case SDLK_F7:        return KeyCode::F7;
        case SDLK_F8:        return KeyCode::F8;
        case SDLK_F9:        return KeyCode::F9;
        case SDLK_F10:       return KeyCode::F10;
        case SDLK_F11:       return KeyCode::F11;
        case SDLK_F12:       return KeyCode::F12;
        case SDLK_LSHIFT:    return KeyCode::LeftShift;
        case SDLK_LCTRL:     return KeyCode::LeftControl;
        case SDLK_LALT:      return KeyCode::LeftAlt;
        case SDLK_RSHIFT:    return KeyCode::RightShift;
        case SDLK_RCTRL:     return KeyCode::RightControl;
        case SDLK_RALT:      return KeyCode::RightAlt;
        default:             return static_cast<KeyCode>(0);
    }
}

void SDL2Window::OnUpdate() {
    SDL_Event sdlEvent;
    while (SDL_PollEvent(&sdlEvent)) {
        if (!m_Data.EventCallback) continue;

        switch (sdlEvent.type) {
            case SDL_QUIT: {
                WindowCloseEvent e;
                m_Data.EventCallback(e);
                break;
            }
            case SDL_WINDOWEVENT: {
                if (sdlEvent.window.event == SDL_WINDOWEVENT_RESIZED) {
                    WindowResizeEvent e(
                        static_cast<u32>(sdlEvent.window.data1),
                        static_cast<u32>(sdlEvent.window.data2)
                    );
                    m_Data.Width  = e.GetWidth();
                    m_Data.Height = e.GetHeight();
                    m_Data.EventCallback(e);
                }
                break;
            }
            case SDL_KEYDOWN: {
                KeyCode key = SDLKeysymToGE(sdlEvent.key.keysym.sym);
                bool repeat  = sdlEvent.key.repeat != 0;
                KeyPressedEvent e(key, repeat);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_KEYUP: {
                KeyCode key = SDLKeysymToGE(sdlEvent.key.keysym.sym);
                KeyReleasedEvent e(key);
                m_Data.EventCallback(e);
                break;
            }
            case SDL_TEXTINPUT: {
                KeyTypedEvent e(static_cast<KeyCode>(sdlEvent.text.text[0]));
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEMOTION: {
                MouseMovedEvent e(
                    static_cast<f32>(sdlEvent.motion.x),
                    static_cast<f32>(sdlEvent.motion.y)
                );
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEWHEEL: {
                MouseScrolledEvent e(
                    static_cast<f32>(sdlEvent.wheel.x),
                    static_cast<f32>(sdlEvent.wheel.y)
                );
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEBUTTONDOWN: {
                MouseButtonPressedEvent e(static_cast<MouseButton>(sdlEvent.button.button));
                m_Data.EventCallback(e);
                break;
            }
            case SDL_MOUSEBUTTONUP: {
                MouseButtonReleasedEvent e(static_cast<MouseButton>(sdlEvent.button.button));
                m_Data.EventCallback(e);
                break;
            }
        }
    }
}

void SDL2Window::SetVSync(bool enabled) {
    // SDL_RENDERER_PRESENTVSYNC is set at renderer creation time;
    // we store the flag here and the renderer backend reads it.
    m_Data.VSync = enabled;
}

} // namespace GE
