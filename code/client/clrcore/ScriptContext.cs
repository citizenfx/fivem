using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

using CitizenFX.Core.Native;

#if USE_HYPERDRIVE
using ContextType = CitizenFX.Core.RageScriptContext;
#else
using ContextType = CitizenFX.Core.fxScriptContext;
#endif

namespace CitizenFX.Core
{
	public static class ScriptContext
	{
		private static readonly ConcurrentQueue<Action> ms_finalizers = new ConcurrentQueue<Action>();

		private static readonly object ms_lock = new object();

		internal static object Lock => ms_lock;

		[ThreadStatic]
		internal static ContextType m_extContext = new ContextType();

		[SecuritySafeCritical]
		public static void Reset()
		{
			InternalReset();
		}

		[SecurityCritical]
		private static void InternalReset()
		{
			m_extContext.numArguments = 0;
			m_extContext.numResults = 0;
			//CleanUp();
		}

		[SecuritySafeCritical]
		public static void Push(object arg)
		{
			PushInternal(arg);
		}

		[SecurityCritical]
		private unsafe static void PushInternal(object arg)
		{
			fixed (ContextType* context = &m_extContext)
			{
				Push(context, arg);
			}
		}

		[SecurityCritical]
		internal unsafe static void Push(ContextType* context, object arg)
		{
			if (arg == null)
			{
				arg = 0;
			}

			if (arg.GetType().IsEnum)
			{
				arg = Convert.ChangeType(arg, arg.GetType().GetEnumUnderlyingType());
			}

			if (arg is string)
			{
				var str = (string)Convert.ChangeType(arg, typeof(string));
				PushString(context, str);

				return;
			}
			else if (arg is Vector3 v)
			{
				PushFast(context, v);

				return;
			}
			else if (arg is InputArgument ia)
			{
				Push(context, ia.Value);

				return;
			}
			else if (Marshal.SizeOf(arg.GetType()) <= 8)
			{
				PushUnsafe(context, arg);
			}

			context->numArguments++;
		}

		[SecurityCritical]
		internal static unsafe void PushUnsafe(ContextType* cxt, object arg)
		{
			*(long*)(&cxt->functionData[8 * cxt->numArguments]) = 0;
			Marshal.StructureToPtr(arg, new IntPtr(cxt->functionData + (8 * cxt->numArguments)), false);
		}

		[SecuritySafeCritical]
		internal static unsafe void PushFast<T>(ContextType* cxt, T arg)
			where T : struct
		{
			var size = FastStructure<T>.Size;

			var numArgs = (size / 8);

			if ((size % 8) != 0)
			{
				*(long*)(&cxt->functionData[8 * cxt->numArguments]) = 0;
				numArgs++;
			}

			FastStructure<T>.StructureToPtr(ref arg, new IntPtr(&cxt->functionData[8 * cxt->numArguments]));

			cxt->numArguments += numArgs;
		}

		[SecurityCritical]
		internal static unsafe T GetResultFast<T>(ContextType* cxt)
			where T : struct
		{
			return FastStructure<T>.PtrToStructure(new IntPtr(&cxt->functionData));
		}

		[SecurityCritical]
		internal unsafe static void PushString(string str)
		{
			fixed (ContextType* cxt = &m_extContext)
			{
				PushString(cxt, str);
			}
		}

		[SecurityCritical]
		internal unsafe static void PushString(ContextType* cxt, string str)
		{
			var ptr = IntPtr.Zero;

			if (str != null)
			{
				var b = Encoding.UTF8.GetBytes(str);

				ptr = Marshal.AllocHGlobal(b.Length + 1);

				Marshal.Copy(b, 0, ptr, b.Length);
				Marshal.WriteByte(ptr, b.Length, 0);

				ms_finalizers.Enqueue(() => Free(ptr));
			}

			unsafe
			{
				*(IntPtr*)(&cxt->functionData[8 * cxt->numArguments]) = ptr;
			}

			cxt->numArguments++;
		}

		[SecuritySafeCritical]
		private static void Free(IntPtr ptr)
		{
			Marshal.FreeHGlobal(ptr);
		}

		[SecuritySafeCritical]
		public static T GetResult<T>()
		{
			return (T)GetResult(typeof(T));
		}

		[SecuritySafeCritical]
		public static object GetResult(Type type)
		{
			return GetResultHelper(type);
		}

		[SecurityCritical]
		internal unsafe static object GetResult(ContextType* cxt, Type type)
		{
			return GetResultHelper(cxt, type);
		}

		[SecurityCritical]
		private static unsafe object GetResultHelper(Type type)
		{
			fixed (ContextType* cxt = &m_extContext)
			{
				return GetResultHelper(cxt, type);
			}
		}

		[SecurityCritical]
		private static unsafe object GetResultHelper(ContextType* context, Type type)
		{
			return GetResult(type, &context->functionData[0]);
		}

		[SecurityCritical]
		internal unsafe static object GetResult(Type type, byte* ptr)
		{
			if (type == typeof(string))
			{
				var nativeUtf8 = *(IntPtr*)&ptr[0];

				if (nativeUtf8 == IntPtr.Zero)
				{
					return null;
				}

				var len = 0;
				while (Marshal.ReadByte(nativeUtf8, len) != 0)
				{
					++len;
				}

				var buffer = new byte[len];
				Marshal.Copy(nativeUtf8, buffer, 0, buffer.Length);
				return Encoding.UTF8.GetString(buffer);
			}

			if (type == typeof(object))
			{
				var dataPtr = *(IntPtr*)&ptr[0];
				var dataLength = *(long*)&ptr[8];

				byte[] data = new byte[dataLength];
				Marshal.Copy(dataPtr, data, 0, (int)dataLength);

				return MsgPackDeserializer.Deserialize(data);
			}

			if (type.IsEnum)
			{
				return Enum.ToObject(type, (int)GetResult(typeof(int), ptr));
			}

			if (type.IsAssignableFrom(typeof(INativeValue)))
			{
				var a = (int)GetResultInternal(typeof(int), ptr);

				return Activator.CreateInstance(type, a);
			}

			if (type == typeof(Vector3))
			{
				var x = *(float*)(&ptr[0]);
				var y = *(float*)(&ptr[8]);
				var z = *(float*)(&ptr[16]);

				return new Vector3(x, y, z);
			}

			if (Marshal.SizeOf(type) <= 8)
			{
				return GetResultInternal(type, ptr);
			}

			return null;
		}
		
		[SecurityCritical]
		private static unsafe object GetResultInternal(Type type, byte* ptr)
		{
			return Marshal.PtrToStructure(new IntPtr(ptr), type);
		}

#if USE_HYPERDRIVE
		internal unsafe delegate bool CallFunc(void* cxtRef);

		[ThreadStatic]
		private static Dictionary<ulong, CallFunc> ms_invokers = new Dictionary<ulong, CallFunc>();

		private static long ms_nativeInvokeFn = GetProcAddress(LoadLibrary("scripting-gta.dll"), "WrapNativeInvoke").ToInt64();

		[SecurityCritical]
		internal unsafe static CallFunc DoGetNative(ulong native)
		{
			return BuildFunction(native, GetNative(native));
		}

		[SecurityCritical]
		private unsafe static CallFunc BuildFunction(ulong native, ulong ptr)
		{
			var method = new DynamicMethod($"NativeCallFn_{ptr}",
				typeof(bool), new Type[1] { typeof(void*) }, typeof(ScriptContext).Module);

			ILGenerator generator = method.GetILGenerator();
			generator.Emit(OpCodes.Ldc_I8, (long)ptr);
			generator.Emit(OpCodes.Ldc_I8, (long)native);
			generator.Emit(OpCodes.Ldarg_0);
			generator.Emit(OpCodes.Ldc_I8, ms_nativeInvokeFn);
			generator.EmitCalli(OpCodes.Calli, CallingConventions.Standard, typeof(bool), new Type[3] { typeof(void*), typeof(ulong), typeof(void*) }, null);
			generator.Emit(OpCodes.Ret);

			return (CallFunc)method.CreateDelegate(typeof(CallFunc));
		}

#if IS_RDR3
		[DllImport("rage-scripting-rdr3.dll", EntryPoint = "?GetNativeHandler@scrEngine@rage@@SAP6AXPEAVscrNativeCallContext@2@@Z_K@Z")]
#else
		[DllImport("rage-scripting-five.dll", EntryPoint = "?GetNativeHandler@scrEngine@rage@@SAP6AXPEAVscrNativeCallContext@2@@Z_K@Z")]
#endif
		private static extern ulong GetNative(ulong hash);

		[DllImport("kernel32.dll")]
		private static extern IntPtr LoadLibrary(string dllToLoad);

		[DllImport("kernel32.dll")]
		private static extern IntPtr GetProcAddress(IntPtr hModule, string procedureName);
#endif

		[SecurityCritical]
		internal unsafe static void Invoke(ContextType* cxt, ulong nativeIdentifier) => InvokeInternal(cxt, nativeIdentifier, InternalManager.ScriptHost);

		internal static void Invoke(ulong nativeIdentifier) => Invoke(nativeIdentifier, InternalManager.ScriptHost);

		[SecuritySafeCritical]
		internal static void Invoke(ulong nativeIdentifier, IScriptHost scriptHost) => InvokeInternal(nativeIdentifier, scriptHost);

		[SecurityCritical]
		private unsafe static void InvokeInternal(ulong nativeIdentifier, IScriptHost scriptHost)
		{
			var context = m_extContext;
			m_extContext = new ContextType();

			ContextType* cxt = &context;
			InvokeInternal(cxt, nativeIdentifier, scriptHost);

			m_extContext = context;
		}

		[SecurityCritical]
		private unsafe static void InvokeInternal(ContextType* cxt, ulong nativeIdentifier, IScriptHost scriptHost)
		{
			cxt->nativeIdentifier = nativeIdentifier;

			unsafe
			{
#if !USE_HYPERDRIVE
				scriptHost.InvokeNative(new IntPtr(cxt));
#else
				if (!ms_invokers.TryGetValue(nativeIdentifier, out CallFunc invoker))
				{
					invoker = BuildFunction(nativeIdentifier, GetNative(nativeIdentifier));
					ms_invokers.Add(nativeIdentifier, invoker);
				}

				cxt->functionDataPtr = &cxt->functionData[0];
				cxt->retDataPtr = &cxt->functionData[0];

				if (!invoker(cxt))
				{
					throw new Exception("Native invocation failed.");
				}

				CopyReferencedParametersOut(cxt);
#endif
			}
		}

#if USE_HYPERDRIVE
		[SecurityCritical]
		internal unsafe static void CopyReferencedParametersOut(ContextType* cxt)
		{
			uint result = 0;
			long a1 = (long)cxt;

			for (; *(uint*)(a1 + 24) != 0; *(uint*)(*(ulong*)(a1 + 8 * *(int *)(a1 + 24) + 32) + 16) = result)
			{
				--*(uint*)(a1 + 24);
				**(uint**)(a1 + 8 * *(int*)(a1 + 24) + 32) = *(uint*)(a1 + 16 * (*(int*)(a1 + 24) + 4));
				*(uint*)(*(ulong*)(a1 + 8 * *(int*)(a1 + 24) + 32) + 8) = *(uint*)(a1
					+ 16
					* *(int*)(a1 + 24)
					+ 68);
				result = *(uint*)(a1 + 16 * *(int*)(a1 + 24) + 72);
			}

			--*(uint*)(a1 + 24);
		}
#endif

		internal static void GlobalCleanUp()
		{
			lock (ms_lock)
			{
				while (ms_finalizers.TryDequeue(out var cb))
				{
					cb();
				}
			}
		}
	}
}
