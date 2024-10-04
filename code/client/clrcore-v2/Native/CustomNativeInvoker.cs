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
	[SecuritySafeCritical]
	public static class CustomNativeInvoker
	{
		const int BufferCount = 32;

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
			[SecurityCritical]
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

#if !IS_FXSERVER
					if (m_ctx.initialArguments != null)
					{
						// Buffer.MemoryCopy not available on client
						for (uint i = 0; i < BufferCount; ++i)
						{
							m_ctx.initialArguments[i] = m_ctx.functionDataPtr[i];
						}
					}
#endif

					ScriptInterface.InvokeNative(handle, ref m_ctx, m_nativeHash);
				}
			}
		}

		public static unsafe T Call<T>(ulong hash, params Argument[] arguments)
		{
			ulong* __data = stackalloc ulong[32];
#if !IS_FXSERVER
			ulong* initialValues = stackalloc ulong[32]; // stackalloc needs to go with lvalue, otherwise the compiler is complaining
#else
			ulong* initialValues = null;
#endif

			int numResults = InvokeInternal(__data, hash, arguments, initialValues);

#if !IS_FXSERVER
			if ((typeof(T) == typeof(CString) || typeof(T) == typeof(string)) && __data[0] != 0)
			{
				NativeStringResultSanitization(hash, arguments, __data, numResults, initialValues);
			}
#endif
			return (T)ScriptContext.GetResult(typeof(T), __data, hash);
		}

		public static unsafe void Call(ulong hash, params Argument[] arguments)
		{
			ulong* __data = stackalloc ulong[32];
			InvokeInternal(__data, hash, arguments, null);
		}

		private static unsafe int InvokeInternal(ulong* data, ulong nativeHash, Argument[] args, ulong* initialData)
		{
			CustomInvocation invoker = default;
			invoker.m_nativeHash = nativeHash;
			invoker.m_args = args;
			invoker.m_length = args.Length;
			invoker.m_ctx.Initialize(data, 0);
			invoker.m_ctx.initialArguments = initialData;

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

			return invoker.m_ctx.numArguments;
		}

		/// <summary>
		/// Sanitization for string result types
		/// Loops through all values given by the ScRT and deny any that equals the result value which isn't of the string type
		/// </summary>
		/// <returns>Result from <see cref="ScriptContext.GetResult(Type, ulong*, ulong)"/> or null if sanitized</returns>
		private static unsafe void NativeStringResultSanitization(ulong hash, Argument[] inputArguments, ulong* arguments, int numArguments, ulong* initialArguments)
		{
			var resultValue = arguments[0];

			// Step 1: quick compare all values until we found a hit
			// By not switching between all the buffers (incl. input arguments) we'll not introduce unnecessary cache misses.
			for (int a = 0; a < numArguments; ++a)
			{
				if (initialArguments[a] == resultValue)
				{
					// Step 2: loop our input list for as many times as `a` was increased
					int inputSize = inputArguments.Length;
					for (int i = 0; i < inputSize; ++i)
					{
						var csArg = inputArguments[i];

						// `a` can be reused by simply decrementing it, we'll go negative when we hit our goal as we decrement before checking (e.g.: `0 - 1 = -1` or `0 - 4 = -4`)
						switch (csArg)
						{
							/*case Input.Vector2 v2: // not available on v2 yet
								a -= 2;
								break;*/
							case Input.Vector3 v3:
								a -= 3;
								break;
							case Input.Vector4 v4:
								a -= 4;
								break;
							default:
								a--;
								break;
						}

						// string type is allowed
						if (a < 0)							
						{
							if (csArg?.GetType() != typeof(Input.CString))
							{
								Debug.WriteLine($"Warning: Sanitized coerced string result for native 0x{hash:X16}");
								arguments[0] = 0;
							}							

							return; // we found our arg, no more to check
						}
					}

					return; // found our value, no more to check
				}
			}
		}
	}
}
