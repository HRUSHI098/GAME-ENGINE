#pragma once

#include "GE/Core/Base.h"
#include "GE/Core/Events/KeyEvents.h"
#include "GE/Core/Events/MouseEvents.h"

namespace GE {

// Static input query API — poll key/mouse state each frame.
// Implemented per-platform (SDL2 backend reads SDL_GetKeyboardState / SDL_GetMouseState).
class Input {
public:
    static bool IsKeyPressed(KeyCode key);
    static bool IsMouseButtonPressed(MouseButton button);
    static f32  GetMouseX();
    static f32  GetMouseY();
    static std::pair<f32, f32> GetMousePosition();
};

} // namespace GE
