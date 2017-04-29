using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Sequential)]
	[Serializable]
	public struct fxScriptContext
	{
		[MarshalAs(UnmanagedType.ByValArray, SizeConst = 32 * 8)]
		public byte[] functionData;

		public int numArguments;
		public int numResults;

		public ulong nativeIdentifier;
	}
}