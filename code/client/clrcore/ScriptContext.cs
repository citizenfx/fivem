using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public class ScriptContext : IDisposable
	{
		private fxScriptContext m_context;
		private static readonly List<Action> ms_finalizers = new List<Action>();

		public ScriptContext()
		{
			Reset();
		}

		public void Reset()
		{
			//CleanUp();

			m_context = new fxScriptContext();
			m_context.functionData = new byte[32 * 8];
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
				var b = Encoding.UTF8.GetBytes(str);

				var ptr = Marshal.AllocHGlobal(b.Length + 1);

				Marshal.Copy(b, 0, ptr, b.Length);
				Marshal.WriteByte(ptr, b.Length, 0);
                
				ms_finalizers.Add(() => Free(ptr));

				b = BitConverter.GetBytes(ptr.ToInt64());
				Array.Copy(b, 0, m_context.functionData, 8 * m_context.numArguments, 8);
			}
			else if (arg is Vector3 v)
			{
				var start = 8 * m_context.numArguments;
				Array.Clear(m_context.functionData, start, 8 * 3);
				Array.Copy(BitConverter.GetBytes(v.X), 0, m_context.functionData, start, 4);
				Array.Copy(BitConverter.GetBytes(v.Y), 0, m_context.functionData, start + 8, 4);
				Array.Copy(BitConverter.GetBytes(v.Z), 0, m_context.functionData, start + 16, 4);

				m_context.numArguments += 2;
			}
			else if (Marshal.SizeOf(arg.GetType()) <= 8)
			{
				var ptr = Marshal.AllocHGlobal(8);
				try
				{
					Marshal.WriteInt64(ptr, 0, 0);
					Marshal.StructureToPtr(arg, ptr, false);

					Marshal.Copy(ptr, m_context.functionData, 8 * m_context.numArguments, 8);
				}
				finally
				{
					Marshal.FreeHGlobal(ptr);
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
			return GetResult(type, m_context.functionData);
		}

		[SecurityCritical]
		internal static object GetResult(Type type, byte[] ptr)
		{
			if (type == typeof(string))
			{
				var nativeUtf8 = new IntPtr(BitConverter.ToInt64(ptr, 0));

				var len = 0;
				while (Marshal.ReadByte(nativeUtf8, len) != 0)
				{
					++len;
				}

				var buffer = new byte[len];
				Marshal.Copy(nativeUtf8, buffer, 0, buffer.Length);
				return Encoding.UTF8.GetString(buffer);
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

		[SecuritySafeCritical]
		public void Invoke(ulong nativeIdentifier, IScriptHost scriptHost) => InvokeInternal(nativeIdentifier, scriptHost);

		[SecurityCritical]
		private void InvokeInternal(ulong nativeIdentifier, IScriptHost scriptHost)
		{
			m_context.nativeIdentifier = nativeIdentifier;

			scriptHost.InvokeNative(ref m_context);
		}


		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		protected virtual void Dispose(bool disposing)
		{
			if (disposing)
			{
                
			}

			//CleanUp();
		}

		internal static void GlobalCleanUp()
		{
			ms_finalizers?.ForEach(a => a());
			ms_finalizers?.Clear();
		}
	}
}