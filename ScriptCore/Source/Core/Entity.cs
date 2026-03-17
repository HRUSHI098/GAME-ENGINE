namespace GE
{
    /// <summary>
    /// Base class for all C# scripts. Attach to an entity via ScriptComponent.
    /// Override OnCreate, OnUpdate, OnDestroy as needed.
    /// </summary>
    public abstract class Entity
    {
        // Set by the engine before OnCreate is called.
        public uint ID { get; internal set; }

        protected Entity() { ID = 0; }

        // ── Lifecycle ──────────────────────────────────────────────────────────
        public virtual void OnCreate()              { }
        public virtual void OnUpdate(float deltaTime){ }
        public virtual void OnDestroy()             { }

        // ── Component access ──────────────────────────────────────────────────
        public bool HasComponent<T>() where T : Component
            => HasComponent_Native(ID, typeof(T));

        public T GetComponent<T>() where T : Component, new()
        {
            if (!HasComponent<T>())
                throw new System.InvalidOperationException(
                    $"Entity {ID} does not have component {typeof(T).Name}");

            var component = new T();
            component.OwnerID = ID;
            return component;
        }

        // ── Entity lookup ─────────────────────────────────────────────────────
        public static Entity? FindByName(string name)
        {
            uint id = FindEntityByName_Native(name);
            if (id == uint.MaxValue) return null;
            // The engine returns a fully populated entity via ScriptEngine
            return null;   // placeholder — engine populates this
        }

        // ── Input shortcut ────────────────────────────────────────────────────
        protected bool IsKeyPressed(KeyCode key) => Input.IsKeyPressed(key);

        // ── Internals ─────────────────────────────────────────────────────────
        [System.Runtime.CompilerServices.MethodImpl(
            System.Runtime.CompilerServices.MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(uint entityID, System.Type componentType);

        [System.Runtime.CompilerServices.MethodImpl(
            System.Runtime.CompilerServices.MethodImplOptions.InternalCall)]
        private static extern uint FindEntityByName_Native(string name);
    }
}
