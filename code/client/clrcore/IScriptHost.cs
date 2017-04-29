using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("8ffdc384-4767-4ea2-a935-3bfcad1db7bf")]
	[ComImport]
	public interface IScriptHost
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		void InvokeNative([MarshalAs(UnmanagedType.Struct)] ref fxScriptContext context);

		[MethodImpl(MethodImplOptions.InternalCall)]
		[return: MarshalAs(UnmanagedType.Interface)]
		fxIStream OpenSystemFile([MarshalAs(UnmanagedType.LPStr)] string fileName);

		[MethodImpl(MethodImplOptions.InternalCall)]
		[return: MarshalAs(UnmanagedType.Interface)]
		fxIStream OpenHostFile([MarshalAs(UnmanagedType.LPStr)] string fileName);

		[MethodImpl(MethodImplOptions.InternalCall)]
		IntPtr CanonicalizeRef(int localRef, int instanceId);

		[MethodImpl(MethodImplOptions.InternalCall)]
		[PreserveSig]
		void ScriptTrace([MarshalAs(UnmanagedType.LPStr)] string message);
	}
}