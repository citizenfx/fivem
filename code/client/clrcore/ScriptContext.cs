using System;
using System.Collections.Concurrent;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public class ScriptContext
	{
		private static readonly ConcurrentQueue<Action> ms_finalizers = new ConcurrentQueue<Action>();

		[ThreadStatic]
		internal static fxScriptContext m_context = new fxScriptContext();

		public ScriptContext()
		{
			Reset();
		}

		[SecuritySafeCritical]
		public void Reset()
		{
			InternalReset();
		}

		[SecurityCritical]
		private void InternalReset()
		{
			m_context.numArguments = 0;
			m_context.numResults = 0;
			//CleanUp();
		}

		[SecuritySafeCritical]
		public void Push(object arg)
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
				PushString(str);

				return;
			}
			else if (arg is Vector3 v)
			{
				PushFast(v);

				return;
			}
			else if (arg is InputArgument ia)
			{
				Push(ia.Value);

				return;
			}
			else if (Marshal.SizeOf(arg.GetType()) <= 8)
			{
				PushUnsafe(arg);
			}

			m_context.numArguments++;
		}

		[SecurityCritical]
		internal unsafe void PushUnsafe(object arg)
		{
			fixed (byte* functionData = m_context.functionData)
			{
				*(long*)(&functionData[8 * m_context.numArguments]) = 0;
				Marshal.StructureToPtr(arg, new IntPtr(functionData + (8 * m_context.numArguments)), false);
			}
		}

		[SecuritySafeCritical]
		internal unsafe void PushFast<T>(T arg)
			where T : struct
		{
			var size = FastStructure<T>.Size;

			var numArgs = (size / 8);

			fixed (byte* functionData = m_context.functionData)
			{
				if ((size % 8) != 0)
				{
					*(long*)(&functionData[8 * m_context.numArguments]) = 0;
					numArgs++;
				}

				FastStructure<T>.StructureToPtr(ref arg, new IntPtr(&functionData[8 * m_context.numArguments]));
			}

			m_context.numArguments += numArgs;
		}

		[SecurityCritical]
		internal unsafe T GetResultFast<T>()
			where T : struct
		{
			fixed (byte* functionData = m_context.functionData)
			{
				return FastStructure<T>.PtrToStructure(new IntPtr(functionData));
			}
		}

		[SecurityCritical]
		internal void PushString(string str)
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
				fixed (byte* functionData = m_context.functionData)
				{
					*(IntPtr*)(&functionData[8 * m_context.numArguments]) = ptr;
				}
			}

			m_context.numArguments++;
		}

		[SecuritySafeCritical]
		private static void Free(IntPtr ptr)
		{
			Marshal.FreeHGlobal(ptr);
		}

		[SecuritySafeCritical]
		public T GetResult<T>()
		{
			return (T)GetResult(typeof(T));
		}

		[SecuritySafeCritical]
		public object GetResult(Type type)
		{
			return GetResultHelper(type);
		}

		[SecurityCritical]
		private unsafe object GetResultHelper(Type type)
		{
			var bytes = new byte[3 * 8];

			fixed (byte* inPtr = m_context.functionData)
			{
				fixed (byte* outPtr = bytes)
				{
#if IS_FXSERVER
					Buffer.MemoryCopy(inPtr, outPtr, bytes.Length, 24);
#else
					UnsafeNativeMethods.CopyMemoryPtr(outPtr, inPtr, 24);
#endif
				}
			}

			return GetResult(type, bytes);
		}

		[SecurityCritical]
		internal static object GetResult(Type type, byte[] ptr)
		{
			if (type == typeof(string))
			{
				var nativeUtf8 = new IntPtr(BitConverter.ToInt64(ptr, 0));

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
				var dataPtr = new IntPtr(BitConverter.ToInt64(ptr, 0));
				var dataLength = BitConverter.ToInt64(ptr, 8);

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
				var x = BitConverter.ToSingle(ptr, 0);
				var y = BitConverter.ToSingle(ptr, 8);
				var z = BitConverter.ToSingle(ptr, 16);

				return new Vector3(x, y, z);
			}

			if (Marshal.SizeOf(type) <= 8)
			{
				return GetResultInternal(type, ptr);
			}

			return null;
		}

		[SecurityCritical]
		private static unsafe object GetResultInternal(Type type, byte[] ptr)
		{
			fixed (byte* bit = &ptr[0])
			{
				return Marshal.PtrToStructure(new IntPtr(bit), type);
			}
		}

		internal void Invoke(ulong nativeIdentifier) => Invoke(nativeIdentifier, InternalManager.ScriptHost);

		[SecuritySafeCritical]
		public void Invoke(ulong nativeIdentifier, IScriptHost scriptHost) => InvokeInternal(nativeIdentifier, scriptHost);

		[SecurityCritical]
		private void InvokeInternal(ulong nativeIdentifier, IScriptHost scriptHost)
		{
			m_context.nativeIdentifier = nativeIdentifier;

			unsafe
			{
				fixed (fxScriptContext* cxt = &m_context)
				{
					scriptHost.InvokeNative(new IntPtr(cxt));
				}
			}
		}

		internal static void GlobalCleanUp()
		{
			while (ms_finalizers.TryDequeue(out var cb))
			{
				cb();
			}
		}
	}
}
