namespace GE
{
    /// <summary>Base class for all managed component wrappers.</summary>
    public abstract class Component
    {
        // Set by Entity.GetComponent<T>() before the component is returned.
        public uint OwnerID { get; internal set; }
    }
}
