using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
    internal static class GameInterface
    {
        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void fwFree(IntPtr ptr);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PrintLog(string text);

		[SecurityCritical]
		[MethodImpl(MethodImplOptions.InternalCall)]
		public static extern ulong GetMemoryUsage();

		[SecurityCritical]
		[DllImport("CoreRT", EntryPoint = "CoreFxCreateObjectInstance")]
		public static extern int CreateObjectInstance([MarshalAs(UnmanagedType.LPStruct)] Guid clsid, [MarshalAs(UnmanagedType.LPStruct)] Guid iid, out IntPtr objectPtr);
    }
}
