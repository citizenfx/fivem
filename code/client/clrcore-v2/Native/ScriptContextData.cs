using System;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Sequential)]
	[Serializable, SecurityCritical]
	internal struct fxScriptContext // fx::ScriptContext
	{
		public unsafe ulong* functionDataPtr;
		public unsafe ulong* retDataPtr;

		public int numArguments;
		public int numResults;

		public unsafe ulong* initialArguments; // dummy

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal unsafe void Initialize(ulong* data, int argCount)
		{
			functionDataPtr = data;
			retDataPtr = data;
			numArguments = argCount;
			numResults = 0;
		}
	}

	[StructLayout(LayoutKind.Sequential)]
	[Serializable, SecurityCritical]
	internal unsafe struct RageScriptContext // rage::scrNativeCallContext
	{
		public ulong* functionDataPtr;
		public int numArguments;
		public int pad;

		public ulong* retDataPtr;

		public int numResults;
		public int pad2;
		public fixed long copyToVectors[4];
		public fixed float vectorData[4 * 4];

		public fixed byte padding[96];

		public unsafe ulong* initialArguments;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal unsafe void Initialize(ulong* data, int argCount)
		{
			functionDataPtr = retDataPtr = data;
			numArguments = argCount;
			numResults = 0;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal unsafe static void CopyVectorData(RageScriptContext* ctx)
		{
			Vector3** dst = (Vector3**)ctx->copyToVectors;
			Vector4* src = (Vector4*)ctx->vectorData;
			Vector4* end = src + ctx->numResults;

			for (; src < end; src++)
				**dst++ = (Vector3)(*src);
		}
	}
}
