using System;

#if REMOTE_FUNCTION_ENABLED
namespace CitizenFX.Core
{
	public delegate object RemoteFunction(params object[] args);

	internal sealed class _RemoteFunction : _ExternalFunction
	{
		private string m_target;

		internal _RemoteFunction(string reference, string target, bool repeatable)
			: base(reference, repeatable)
		{
#if IS_FXSERVER
			m_target = new Player(target).Handle;
#else
			m_target = target;
#endif
		}

		internal static Callback Create(string reference, string netSource)
		{
			bool reply = false, repeatable = false;
			if (ulong.TryParse(reference, out ulong u64Reference))
			{
				reply = (u64Reference & (ulong)Flags.HAS_RETURN_VALUE) != 0;
				repeatable = (u64Reference & (ulong)Flags.IS_PERSISTENT) != 0;
			}

			var remoteFunction = new _RemoteFunction(reference, netSource, repeatable);
			return reply ? new Callback(remoteFunction.Invoke) : new Callback(remoteFunction.InvokeNoReturn);
		}

#if IS_FXSERVER
		private void EventTrigger(string eventName, object[] arg) => Events.TriggerClientEvent(eventName, m_target, arg);
#else
		private void EventTrigger(string eventName, object[] arg) => Events.TriggerServerEvent(eventName, arg);
#endif

		private Coroutine<object> InvokeNoReturn(object[] args)
		{
			EventTrigger(ExternalsManager.RpcRequestName, new object[] { ScriptInterface.ResourceName, ulong.MaxValue, m_reference, args });
			return Coroutine<object>.Completed(null);
		}

		private Coroutine<object> Invoke(object[] args)
		{
			var promise = new Promise<object>();
			ulong callbackId = ExternalsManager.RegisterRemoteFunction(typeof(object), new DynFunc((resultArgs) =>
			{
				if (resultArgs.Length > 1 && resultArgs[1] != null)
					promise.Complete(new Exception(resultArgs[1].ToString()));
				else
					promise.Complete(resultArgs[0]);

				return null;
			}));

#if IS_FXSERVER
			ExternalsManager.AddRPCToPlayer(m_target, (int)callbackId);
#endif
			EventTrigger(ExternalsManager.RpcRequestName, new object[] { ScriptInterface.ResourceName, callbackId, m_reference, args });

			return promise;
		}
	}
}
#endif
