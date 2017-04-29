using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("a2f1b24b-a29f-4121-8162-86901eca8097")]
	public interface IScriptRefRuntime
	{
		void CallRef(int refIndex,
					 [In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] argsSerialized,
					 int argsSize,
					 [Out] out IntPtr retvalSerialized,
					 [Out] out int retvalSize);

		int DuplicateRef(int refIndex);

		void RemoveRef(int refIndex);
	}
}