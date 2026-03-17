using System.Runtime.CompilerServices;

namespace GE
{
    public static class Log
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Trace(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Info(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Warn(string message);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void Error(string message);
    }
}
