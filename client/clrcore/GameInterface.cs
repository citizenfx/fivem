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
        public static extern IntPtr OpenFile(string fileName);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int ReadFile(IntPtr handle, byte[] buffer, int offset, int length);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void CloseFile(IntPtr handle);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern int GetFileLength(IntPtr handle);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool GetEnvironmentInfo(out string resourceName, out string resourcePath, out string resourceAssembly, out uint instanceId);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void PrintLog(string text);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern byte[] InvokeNativeReference(string resource, uint instance, uint reference, byte[] argsSerialized);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void DeleteNativeReference(string resource, uint instance, uint reference);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void TriggerEvent(string eventName, byte[] argsSerialized, bool isRemote);

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern byte[] InvokeResourceExport(string resource, string exportName, byte[] argsSerialized);

        [StructLayout(LayoutKind.Sequential)]
        public struct NativeCallArguments
        {
	        uint nativeHash;
	        int numArguments;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst=16)]
	        uint[] intArguments;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst=16)]
	        float[] floatArguments;

            [MarshalAs(UnmanagedType.ByValArray, SizeConst=16)]
	        byte[] argumentFlags;
	        
            uint resultValue;
        }

        [SecurityCritical]
        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern bool InvokeGameNative(ref NativeCallArguments arguments);
    }
}
