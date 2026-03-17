using System.Runtime.CompilerServices;

namespace GE
{
    public class TransformComponent : Component
    {
        public Vector2 Position
        {
            get { GetPosition_Native(OwnerID, out Vector2 v); return v; }
            set { SetPosition_Native(OwnerID, ref value); }
        }

        public Vector2 Scale
        {
            get { GetScale_Native(OwnerID, out Vector2 v); return v; }
            set { SetScale_Native(OwnerID, ref value); }
        }

        public float Rotation
        {
            get => GetRotation_Native(OwnerID);
            set => SetRotation_Native(OwnerID, value);
        }

        public void Translate(Vector2 delta)
        {
            Position = new Vector2(Position.X + delta.X, Position.Y + delta.Y);
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void  GetPosition_Native(uint id, out Vector2 pos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void  SetPosition_Native(uint id, ref Vector2 pos);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void  GetScale_Native(uint id, out Vector2 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void  SetScale_Native(uint id, ref Vector2 scale);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern float GetRotation_Native(uint id);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void  SetRotation_Native(uint id, float rotation);
    }
}
