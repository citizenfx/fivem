using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Sequential)]
	[Serializable]
	public unsafe struct fxScriptContext
	{
		public fixed byte functionData[8 * 32];

		public int numArguments;
		public int numResults;

		public ulong nativeIdentifier;
	}
}
