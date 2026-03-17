using System.Runtime.CompilerServices;

namespace GE
{
    public enum KeyCode
    {
        Space = 32,
        A = 65, B, C, D, E, F, G, H, I, J, K, L, M,
        N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        D0 = 48, D1, D2, D3, D4, D5, D6, D7, D8, D9,
        Escape = 256, Enter, Tab, Backspace, Insert, Delete,
        Right, Left, Down, Up,
        F1 = 290, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
        LeftShift = 340, LeftControl, LeftAlt,
        RightShift = 344, RightControl, RightAlt,
    }

    public enum MouseButton { Left = 1, Middle = 2, Right = 3 }

    public static class Input
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsKeyPressed(KeyCode key);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool IsMouseButtonPressed(MouseButton button);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetMousePosition(out float x, out float y);

        public static Vector2 MousePosition {
            get { GetMousePosition(out float x, out float y); return new Vector2(x, y); }
        }
    }
}
