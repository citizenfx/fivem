using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Sequential)]
	[Serializable]
	internal unsafe struct fxScriptContext
	{
		public fixed byte functionData[8 * 32];

		public int numArguments;
		public int numResults;

		public ulong nativeIdentifier;
	}

	[StructLayout(LayoutKind.Sequential)]
	[Serializable]
	internal unsafe struct RageScriptContext
	{
		public void* functionDataPtr;
		public int numArguments;
		public int pad;

		public void* retDataPtr;
		public int numResults;
		public int pad2;

		public fixed byte padding[192];

		public fixed byte functionData[8 * 32];

		public ulong nativeIdentifier;
	}
}
