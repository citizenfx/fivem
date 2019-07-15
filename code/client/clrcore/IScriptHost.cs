using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("8ffdc384-4767-4ea2-a935-3bfcad1db7bf")]
	[ComImport]
	internal interface IScriptHost
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		void InvokeNative([MarshalAs(UnmanagedType.Struct)] IntPtr context);

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

		[MethodImpl(MethodImplOptions.InternalCall)]
		void SubmitBoundaryStart([In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] byte[] boundaryData, int boundarySize);

		[MethodImpl(MethodImplOptions.InternalCall)]
		void SubmitBoundaryEnd([In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] byte[] boundaryData, int boundarySize);
	}

	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("9568df2d-27c8-4b9e-b29d-48272c317084")]
	[ComImport]
	internal interface IScriptHostWithResourceData
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		void GetResourceName(out IntPtr nameString);
	}
}
