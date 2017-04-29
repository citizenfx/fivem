using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("67b28af1-aaf9-4368-8296-f93afc7bde96")]
	public interface IScriptRuntime
	{
		void Create([MarshalAs(UnmanagedType.Interface)] IScriptHost host);

		void Destroy();

		[PreserveSig]
		IntPtr GetParentObject();

		[PreserveSig]
		void SetParentObject(IntPtr ptr);

		[PreserveSig]
		int GetInstanceId();
	}
}