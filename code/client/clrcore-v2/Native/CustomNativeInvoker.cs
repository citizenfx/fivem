using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;

#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;
#else
using ContextType = CitizenFX.Core.RageScriptContext;
#endif

namespace CitizenFX.Core.Native
{
	public static class CustomNativeInvoker
	{

		private static Dictionary<ulong, UIntPtr> s_nativeHandles = new Dictionary<ulong, UIntPtr>();

		internal struct CustomInvocation
		{
			internal ContextType m_ctx;
			internal Argument[] m_args;
			internal int m_offset;
			internal int m_length;
			internal ulong m_nativeHash;

			/// <summary>
			/// Keeps adding arguments onto the argument stack, will recurse when arguments need to be fixed.
			/// This approach is 10x faster than the v1 approach in combination with CString
			/// </summary>
			internal unsafe void PushPinAndInvoke()
			{
				while (m_offset < m_length)
				{
					if (m_args[m_offset++] is Argument input)
						input.PushNativeValue(ref this);
					else
						m_ctx.functionDataPtr[m_ctx.numArguments++] = 0uL;
				}

				// invoke when we've hit the end
				if (m_offset == m_length)
				{
					m_offset++; // make sure the calling code won't invoke as well
					if (!s_nativeHandles.TryGetValue(m_nativeHash, out UIntPtr handle))
					{
						handle = ScriptInterface.GetNative(m_nativeHash);
						s_nativeHandles.Add(m_nativeHash, handle);
					}

					ScriptInterface.InvokeNative(handle, ref m_ctx, m_nativeHash);
				}
			}
		}

		public static unsafe T Call<T>(ulong hash, params Argument[] arguments)
		{
			ulong* __data = stackalloc ulong[32];
			InvokeInternal(__data, hash, arguments);
			return (T)ScriptContext.GetResult(typeof(T), __data, hash);
		}

		public static unsafe void Call(ulong hash, params Argument[] arguments)
		{
			ulong* __data = stackalloc ulong[32];
			InvokeInternal(__data, hash, arguments);
		}

		private static unsafe void InvokeInternal(ulong* data, ulong nativeHash, Argument[] args)
		{
			CustomInvocation invoker = default;
			invoker.m_nativeHash = nativeHash;
			invoker.m_args = args;
			invoker.m_length = args.Length;
			invoker.m_ctx.Initialize(data, 0);

			invoker.PushPinAndInvoke();

#if !IS_FXSERVER
			RageScriptContext.CopyVectorData(&invoker.m_ctx);
#endif
			// output arguments
			invoker.m_offset = 0;
			for (int i = 0; i < args.Length; ++i)
			{
				args[i].PullNativeValue(ref invoker);
			}
		}
	}
}
