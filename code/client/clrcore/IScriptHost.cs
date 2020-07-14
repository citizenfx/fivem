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

		[MethodImpl(MethodImplOptions.InternalCall)]
		IntPtr GetLastErrorText();
	}

	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("9568df2d-27c8-4b9e-b29d-48272c317084")]
	[ComImport]
	internal interface IScriptHostWithResourceData
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		void GetResourceName(out IntPtr nameString);

		[MethodImpl(MethodImplOptions.InternalCall)]
		void GetNumResourceMetaData([MarshalAs(UnmanagedType.LPStr)] string fieldName, out IntPtr numFields);

		[MethodImpl(MethodImplOptions.InternalCall)]
		void GetResourceMetaData([MarshalAs(UnmanagedType.LPStr)] string fieldName, out IntPtr fieldValue);
	}

	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("5e212027-3aad-46d1-97e0-b8bc5ef89e18")]
	[ComImport]
	internal interface IScriptHostWithManifest
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		bool IsManifestVersionBetween([In] IntPtr lowerGuid, [In] IntPtr upperguid);

		[MethodImpl(MethodImplOptions.InternalCall)]
		bool IsManifestVersionV2Between([MarshalAs(UnmanagedType.LPStr)] string lowerBound, [MarshalAs(UnmanagedType.LPStr)] string upperBound);
	}
}
