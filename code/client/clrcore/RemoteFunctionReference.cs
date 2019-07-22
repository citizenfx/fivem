using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security;
using System.Runtime.InteropServices;

using static CitizenFX.Core.Native.API;

namespace CitizenFX.Core
{
	class RemoteFunctionReference : IDisposable
	{
		private readonly string m_reference;

		public RemoteFunctionReference(byte[] reference)
		{
			m_reference = Encoding.UTF8.GetString(reference);

			// increment the refcount
			DuplicateFunctionReference(m_reference);
		}

		~RemoteFunctionReference()
		{
			Dispose(false);
		}

		private bool m_disposed = false;

		public void Dispose()
		{
			Dispose(true);
			GC.SuppressFinalize(this);
		}

		protected virtual void Dispose(bool disposing)
		{
			if (m_disposed)
			{
				return;
			}

			if (disposing)
			{
				// no managed objects to free
			}

			FreeNativeReference();

			m_disposed = true;
		}

		private void FreeNativeReference()
		{
			lock (ScriptContext.Lock)
			{
				DeleteFunctionReference(m_reference);
			}
		}

		public byte[] Duplicate()
		{
			return Encoding.UTF8.GetBytes(DuplicateFunctionReference(m_reference));
		}

		[SecuritySafeCritical]
		public byte[] InvokeNative(byte[] argsSerialized)
		{
			return _InvokeNative(argsSerialized);
		}

		[SecurityCritical]
		private byte[] _InvokeNative(byte[] argsSerialized)
		{
			if (GameInterface.SnapshotStackBoundary(out var b))
			{
				InternalManager.ScriptHost.SubmitBoundaryEnd(b, b.Length);
			}

			try
			{
				IntPtr resBytes;
				long retLength;

				unsafe
				{
					fixed (byte* argsSerializedRef = &argsSerialized[0])
					{
						resBytes = Native.Function.Call<IntPtr>(Native.Hash.INVOKE_FUNCTION_REFERENCE, m_reference, argsSerializedRef, argsSerialized.Length, &retLength);
					}
				}

				var retval = new byte[retLength];
				Marshal.Copy(resBytes, retval, 0, retval.Length);

				return retval;
			}
			finally
			{
				InternalManager.ScriptHost.SubmitBoundaryEnd(null, 0);
			}
		}
	}

	class NetworkFunctionManager
	{
		private static int curTaskId;

		private static Dictionary<string, TaskCompletionSource<object>> m_rpcList = new Dictionary<string, TaskCompletionSource<object>>();
		private static Dictionary<string, HashSet<string>> m_playerPromises = new Dictionary<string, HashSet<string>>();

		internal static void HandleEventTrigger(string eventName, object[] args, string sourceString)
		{
			try
			{
				if (eventName == $"__cfx_rpcRep:{Native.API.GetCurrentResourceName()}")
				{
					var repId = args[0].ToString();
					var arg = args[1] as List<object>;
					var err = args.Length > 2 ? args[2] : null;

					if (m_rpcList.TryGetValue(repId, out var tcs))
					{
						m_rpcList.Remove(repId);

#if IS_FXSERVER
						foreach (var player in m_playerPromises)
						{
							player.Value.Remove(repId);
						}
#endif

						if (arg != null)
						{
							tcs.TrySetResult(arg[0]);
						}
						else if (err != null)
						{
							tcs.TrySetException(new Exception(err.ToString()));
						}
					}
				}
#if IS_FXSERVER
				else if (eventName == "playerDropped")
				{
					if (m_playerPromises.TryGetValue(sourceString, out var set))
					{
						m_playerPromises.Remove(sourceString);

						foreach (var entry in set)
						{
							if (m_rpcList.TryGetValue(entry, out var tcs))
							{
								tcs.TrySetException(new Exception("The target player was dropped."));
							}
						}
					}
				}
#endif
			}
			catch (Exception e)
			{
				Debug.WriteLine("except: {0}", e.ToString());
			}
		}

		public static object CreateReference(byte[] funcRefData, string netSource)
		{
			return new NetworkCallbackDelegate(async delegate (object[] args)
			{
#if IS_FXSERVER
				void EventTrigger(string name, object[] arg)
				{
					var player = new Player(netSource);
					player.TriggerEvent(name, arg);
				}
#else
				void EventTrigger(string name, object[] arg)
				{
					BaseScript.TriggerEvent(name, arg);
				}
#endif

				var tcs = new TaskCompletionSource<object>();

				var taskId = curTaskId;
				++curTaskId;

				var repId = $"{Native.API.GetInstanceId()}:{taskId}";
				m_rpcList[repId] = tcs;

#if IS_FXSERVER
				// add a player promise
				if (!m_playerPromises.ContainsKey(netSource))
				{
					m_playerPromises[netSource] = new HashSet<string>();
				}

				m_playerPromises[netSource].Add(repId);
#endif

				EventTrigger("__cfx_rpcReq", new object[] {
					$"__cfx_rpcRep:{Native.API.GetCurrentResourceName()}",
					repId,
					Encoding.UTF8.GetString(funcRefData),
					args });

				return await tcs.Task;
			});
		}
	}

	public delegate Task<object> NetworkCallbackDelegate(params object[] args);
}
