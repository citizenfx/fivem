using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading;

namespace CitizenFX.Core
{
	internal static class ReferenceFunctionManager
	{
		internal class Function
		{
			public DynFunc m_method;
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

		/// <summary>
		/// Register a delegate to other runtimes and/or our host (reference function)
		/// </summary>
		/// <param name="method">Delagate to register to the external world</param>
		/// <returns>( internalReferenceId, externalReferenceId )</returns>
		/// <remarks>Don't alter the returned value</remarks>
		[SecuritySafeCritical]
		internal static KeyValuePair<int, byte[]> Create(DynFunc method)
		{
			// TODO: change return type to `ValueTuple` once clients support those

			int id = method.Method.GetHashCode();

			// keep incrementing until we find a free spot
			while (s_references.ContainsKey(id))
				unchecked { ++id; }

			byte[] refId = ScriptInterface.CanonicalizeRef(id);
			s_references[id] = new Function(method, refId);

			return new KeyValuePair<int, byte[]>(id, refId);
		}

		internal static int IncrementReference(int reference)
		{
			if (s_references.TryGetValue(reference, out var funcRef))
			{
				Interlocked.Increment(ref funcRef.m_refCount);
				return reference;
			}

			return -1;
		}

		internal static void DecrementReference(int reference)
		{
			if (s_references.TryGetValue(reference, out var funcRef)
				&& Interlocked.Decrement(ref funcRef.m_refCount) <= 0)
			{
				s_references.Remove(reference);
			}
		}

		/// <summary>
		/// Remove reference function by id
		/// </summary>
		/// <param name="referenceId">Internal reference id of the reference to remove</param>
		internal static void Remove(int referenceId)
		{			
			s_references.Remove(referenceId);
		}

		/// <summary>
		/// Remove all reference functions that are targeting a specific object
		/// </summary>
		/// <remarks>Slow, may need to be replaced</remarks>
		/// <param name="target"></param>
		internal static void RemoveAllWithTarget(object target)
		{
			foreach (var entry in s_references)
			{
				if (entry.Value.m_method.Target == target)
				{
					s_references.Remove(entry.Key);
				}
			}
		}

		/// <summary>
		/// Set reference function to another delegate
		/// </summary>
		/// <param name="referenceId">Reference id of the reference to remove</param>
		/// <param name="newFunc">New delegate/method to set the reference function to</param>
		/// <returns><see langword="true"/> if found and changed, <see langword="false"/> otherwise</returns>
		internal static bool SetDelegate(int referenceId, DynFunc newFunc)
		{
			if (s_references.TryGetValue(referenceId, out var refFunc))
			{
				refFunc.m_method = newFunc;
				return true;
			}

			return false;
		}

		internal static int CreateCommand(string command, DynFunc method, bool isRestricted)
		{
			var registration = Create(method);
			Native.CoreNatives.RegisterCommand(command, new Native.InFunc(registration.Value), isRestricted);

			return registration.Key;
		}

		[SecurityCritical]
		internal unsafe static void IncomingCall(int refIndex, byte* argsSerialized, uint argsSize, out byte[] retval)
		{
			try
			{
				retval = Invoke(refIndex, argsSerialized, argsSize);
			}
			catch (Exception e)
			{
				var errorMessage = Debug.CreateError(e.InnerException ?? e, "reference call");

				retval = MsgPackSerializer.Serialize(new object[] { false, errorMessage });
			}
		}

		[SecurityCritical]
		internal unsafe static byte[] Invoke(int reference, byte* arguments, uint argsSize)
		{
			if (s_references.TryGetValue(reference, out var funcRef))
			{
				var args = MsgPackDeserializer.DeserializeArray(arguments, argsSize);

				object result = null;

				try
				{
					// there's no remote invocation support through here
					result = funcRef.m_method(default, args);
				}
				catch (Exception ex)
				{
					var errorMessage = Debug.CreateError(ex, "reference function");

					return MsgPackSerializer.Serialize(new object[] { false, errorMessage });
				}

				if (result is Coroutine coroutine)
				{
					if (coroutine.IsCompleted)
					{
						if (coroutine.Exception != null)
						{
							Debug.Write(coroutine.Exception);
						}

						return MsgPackSerializer.Serialize(new object[] { true, new[] { coroutine.GetResultNonThrowing(), coroutine.Exception?.ToString() } });
					}
					else
					{
						var returnDictionary = new Dictionary<string, object>()
						{
							{ "__cfx_async_retval", new Action<Callback>(asyncResult =>
								coroutine.ContinueWith(() =>
								{
									if (coroutine.Exception != null)
									{
										Debug.Write(coroutine.Exception);
									}

									asyncResult(coroutine.GetResultNonThrowing(), coroutine.Exception?.ToString());
								}))
							}
						};

						return MsgPackSerializer.Serialize(new object[] { true, returnDictionary });
					}
				}

				return MsgPackSerializer.Serialize(new[] { true, result });
			}
			else
			{
				Debug.WriteLine($"No such reference for {reference}.");
			}

			// return nil
			return MsgPackSerializer.Serialize(new object[] { false, null });
		}
	}
}
