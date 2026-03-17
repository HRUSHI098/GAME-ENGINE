#include "GE/Core/Input.h"
#include <SDL2/SDL.h>

namespace GE {

static SDL_Scancode GEKeyToScancode(KeyCode key) {
    using KC = KeyCode;
    switch (key) {
        // Letters (KeyCode uses uppercase ASCII 65-90; SDL scancodes are fixed)
        case KC::A: return SDL_SCANCODE_A; case KC::B: return SDL_SCANCODE_B;
        case KC::C: return SDL_SCANCODE_C; case KC::D: return SDL_SCANCODE_D;
        case KC::E: return SDL_SCANCODE_E; case KC::F: return SDL_SCANCODE_F;
        case KC::G: return SDL_SCANCODE_G; case KC::H: return SDL_SCANCODE_H;
        case KC::I: return SDL_SCANCODE_I; case KC::J: return SDL_SCANCODE_J;
        case KC::K: return SDL_SCANCODE_K; case KC::L: return SDL_SCANCODE_L;
        case KC::M: return SDL_SCANCODE_M; case KC::N: return SDL_SCANCODE_N;
        case KC::O: return SDL_SCANCODE_O; case KC::P: return SDL_SCANCODE_P;
        case KC::Q: return SDL_SCANCODE_Q; case KC::R: return SDL_SCANCODE_R;
        case KC::S: return SDL_SCANCODE_S; case KC::T: return SDL_SCANCODE_T;
        case KC::U: return SDL_SCANCODE_U; case KC::V: return SDL_SCANCODE_V;
        case KC::W: return SDL_SCANCODE_W; case KC::X: return SDL_SCANCODE_X;
        case KC::Y: return SDL_SCANCODE_Y; case KC::Z: return SDL_SCANCODE_Z;
        // Numbers
        case KC::D0: return SDL_SCANCODE_0; case KC::D1: return SDL_SCANCODE_1;
        case KC::D2: return SDL_SCANCODE_2; case KC::D3: return SDL_SCANCODE_3;
        case KC::D4: return SDL_SCANCODE_4; case KC::D5: return SDL_SCANCODE_5;
        case KC::D6: return SDL_SCANCODE_6; case KC::D7: return SDL_SCANCODE_7;
        case KC::D8: return SDL_SCANCODE_8; case KC::D9: return SDL_SCANCODE_9;
        // Navigation / arrows
        case KC::Right:     return SDL_SCANCODE_RIGHT;
        case KC::Left:      return SDL_SCANCODE_LEFT;
        case KC::Down:      return SDL_SCANCODE_DOWN;
        case KC::Up:        return SDL_SCANCODE_UP;
        case KC::Escape:    return SDL_SCANCODE_ESCAPE;
        case KC::Enter:     return SDL_SCANCODE_RETURN;
        case KC::Tab:       return SDL_SCANCODE_TAB;
        case KC::Backspace: return SDL_SCANCODE_BACKSPACE;
        case KC::Insert:    return SDL_SCANCODE_INSERT;
        case KC::Delete:    return SDL_SCANCODE_DELETE;
        case KC::Space:     return SDL_SCANCODE_SPACE;
        // Function keys
        case KC::F1:  return SDL_SCANCODE_F1;  case KC::F2:  return SDL_SCANCODE_F2;
        case KC::F3:  return SDL_SCANCODE_F3;  case KC::F4:  return SDL_SCANCODE_F4;
        case KC::F5:  return SDL_SCANCODE_F5;  case KC::F6:  return SDL_SCANCODE_F6;
        case KC::F7:  return SDL_SCANCODE_F7;  case KC::F8:  return SDL_SCANCODE_F8;
        case KC::F9:  return SDL_SCANCODE_F9;  case KC::F10: return SDL_SCANCODE_F10;
        case KC::F11: return SDL_SCANCODE_F11; case KC::F12: return SDL_SCANCODE_F12;
        // Modifiers
        case KC::LeftShift:   return SDL_SCANCODE_LSHIFT;
        case KC::LeftControl: return SDL_SCANCODE_LCTRL;
        case KC::LeftAlt:     return SDL_SCANCODE_LALT;
        case KC::RightShift:  return SDL_SCANCODE_RSHIFT;
        case KC::RightControl:return SDL_SCANCODE_RCTRL;
        case KC::RightAlt:    return SDL_SCANCODE_RALT;
        default:              return SDL_SCANCODE_UNKNOWN;
    }
}

bool Input::IsKeyPressed(KeyCode key) {
    const u8* state = SDL_GetKeyboardState(nullptr);
    SDL_Scancode sc = GEKeyToScancode(key);
    return sc != SDL_SCANCODE_UNKNOWN && state[sc] != 0;
}

bool Input::IsMouseButtonPressed(MouseButton button) {
    u32 state = SDL_GetMouseState(nullptr, nullptr);
    return (state & SDL_BUTTON(static_cast<int>(button))) != 0;
}

std::pair<f32, f32> Input::GetMousePosition() {
    int x, y;
    SDL_GetMouseState(&x, &y);
    return { static_cast<f32>(x), static_cast<f32>(y) };
}

f32 Input::GetMouseX() { return GetMousePosition().first; }
f32 Input::GetMouseY() { return GetMousePosition().second; }

} // namespace GE
