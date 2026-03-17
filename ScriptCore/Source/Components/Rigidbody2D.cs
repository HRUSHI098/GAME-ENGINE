using System.Runtime.CompilerServices;

namespace GE
{
    public class RigidBody2DComponent : Component
    {
        public Vector2 LinearVelocity
        {
            get { GetLinearVelocity_Native(OwnerID, out Vector2 v); return v; }
            set { SetLinearVelocity_Native(OwnerID, ref value); }
        }

        public void ApplyImpulse(Vector2 impulse, bool wake = true)
            => ApplyLinearImpulse_Native(OwnerID, ref impulse, wake);

        public void ApplyForce(Vector2 force, bool wake = true)
            => ApplyForce_Native(OwnerID, ref force, wake);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetLinearVelocity_Native(uint id, out Vector2 vel);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetLinearVelocity_Native(uint id, ref Vector2 vel);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ApplyLinearImpulse_Native(uint id, ref Vector2 impulse, bool wake);
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void ApplyForce_Native(uint id, ref Vector2 force, bool wake);
    }
}
