using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading;

namespace CitizenFX.Core
{
	static class ReferenceFunctionManager
	{
		internal struct Function
		{
			public readonly DynFunc m_method;
			public readonly byte[] m_refId;
			public int m_refCount;

			public Function(DynFunc method, byte[] id)
			{
				m_method = method;
				m_refId = id;
				m_refCount = 0;
			}
		}

		private static Dictionary<int, Function> s_references = new Dictionary<int, Function>();

		private static IntPtr m_retvalBuffer;
		private static int m_retvalBufferSize;

		// don't alter the returned value
		[SecuritySafeCritical]
		public static byte[] Create(DynFunc method)
		{
			int id = method.Method.GetHashCode();

			// keep incrementing until we find a free spot
			while (s_references.ContainsKey(id))
				unchecked { ++id; }

			byte[] refId = ScriptInterface.CanonicalizeRef(id);
			s_references[id] = new Function(method, refId);

			return refId;
		}

		public static int Duplicate(int reference)
		{
			if (s_references.TryGetValue(reference, out var funcRef))
			{
				Interlocked.Increment(ref funcRef.m_refCount);
				return reference;
			}

			return -1;
		}

		public static void Remove(int reference)
		{
			if (s_references.TryGetValue(reference, out var funcRef)
				&& Interlocked.Decrement(ref funcRef.m_refCount) <= 0)
			{
				s_references.Remove(reference);
			}
		}

		[SecurityCritical]
		internal unsafe static void IncomingCall(int refIndex, byte* argsSerialized, uint argsSize, out IntPtr retvalSerialized, out uint retvalSize)
		{
			try
			{
				var retvalData = Invoke(refIndex, argsSerialized, argsSize);
				if (retvalData != null)
				{
					int length = retvalData.Length;

					if (m_retvalBuffer == IntPtr.Zero)
					{
						m_retvalBufferSize = Math.Max(32768, length);
						m_retvalBuffer = Marshal.AllocHGlobal(m_retvalBufferSize);
					}
					else if (m_retvalBufferSize < length)
					{
						m_retvalBufferSize = length;
						m_retvalBuffer = Marshal.ReAllocHGlobal(m_retvalBuffer, new IntPtr(m_retvalBufferSize));
					}

					Marshal.Copy(retvalData, 0, m_retvalBuffer, length);

					retvalSerialized = m_retvalBuffer;
					retvalSize = (uint)length;

					return;
				}
			}
			catch (Exception e)
			{

				Debug.PrintError(e.InnerException ?? e, "reference call");
			}

			retvalSerialized = IntPtr.Zero;
			retvalSize = 0u;
		}

		[SecurityCritical]
		public unsafe static byte[] Invoke(int reference, byte* arguments, uint argsSize)
		{
			if (s_references.TryGetValue(reference, out var funcRef))
			{
				var args = MsgPackDeserializer.DeserializeArray(arguments, argsSize);

				object result = null;

				try
				{
					result = funcRef.m_method(args);
				}
				catch (Exception ex)
				{
					string argsString = string.Join<string>(", ", args.Select(a => a is null ? "null" : a.GetType().ToString()));
					Debug.PrintError(ex, $"handling function reference: {funcRef.m_method.Method.Name}\n\twith arguments: ({argsString})");
				}

				if (result is Coroutine coroutine)
				{
					if (coroutine.IsCompleted)
					{
						if (coroutine.GetResult() is Callback callback)
							coroutine.ContinueWith(() => callback(coroutine.GetResult(), coroutine.Exception));
						else
							return MsgPackSerializer.Serialize(new[] { coroutine.GetResult() });
					}
					else
					{
						var returnDictionary = new Dictionary<string, object>(1);
						returnDictionary.Add("__cfx_async_retval", new Action<Callback>(asyncResult =>
							{
								coroutine.ContinueWith(new Action(() => asyncResult(coroutine.GetResult(), coroutine.Exception)));
							})
						);

						return MsgPackSerializer.Serialize(new[] { returnDictionary });
					}
				}

				return MsgPackSerializer.Serialize(new[] { result });
			}
			else
			{
				Debug.WriteLine($"No such reference for {reference}.");
			}

			return new byte[] { 0xC0 }; // return nil
		}
	}
}
