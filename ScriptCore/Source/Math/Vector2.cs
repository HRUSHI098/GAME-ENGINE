using System;

namespace GE
{
    public struct Vector2
    {
        public float X, Y;

        public Vector2(float x, float y) { X = x; Y = y; }
        public Vector2(float scalar)     { X = scalar; Y = scalar; }

        public static readonly Vector2 Zero  = new(0f, 0f);
        public static readonly Vector2 One   = new(1f, 1f);
        public static readonly Vector2 Up    = new(0f, 1f);
        public static readonly Vector2 Down  = new(0f, -1f);
        public static readonly Vector2 Left  = new(-1f, 0f);
        public static readonly Vector2 Right = new(1f, 0f);

        public float Length    => (float)Math.Sqrt(X * X + Y * Y);
        public float LengthSqr => X * X + Y * Y;

        public Vector2 Normalized {
            get {
                float len = Length;
                return len > 1e-6f ? new Vector2(X / len, Y / len) : Zero;
            }
        }

        public static Vector2 operator +(Vector2 a, Vector2 b) => new(a.X + b.X, a.Y + b.Y);
        public static Vector2 operator -(Vector2 a, Vector2 b) => new(a.X - b.X, a.Y - b.Y);
        public static Vector2 operator *(Vector2 a, float s)   => new(a.X * s, a.Y * s);
        public static Vector2 operator *(float s, Vector2 a)   => new(a.X * s, a.Y * s);
        public static Vector2 operator /(Vector2 a, float s)   => new(a.X / s, a.Y / s);
        public static Vector2 operator -(Vector2 a)            => new(-a.X, -a.Y);

        public static float Dot(Vector2 a, Vector2 b)      => a.X * b.X + a.Y * b.Y;
        public static float Distance(Vector2 a, Vector2 b) => (a - b).Length;

        public static Vector2 Lerp(Vector2 a, Vector2 b, float t)
            => new(a.X + (b.X - a.X) * t, a.Y + (b.Y - a.Y) * t);

        public override string ToString() => $"({X:F3}, {Y:F3})";
    }
}
