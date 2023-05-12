using System;

#if REMOTE_FUNCTION_ENABLED
namespace CitizenFX.Core
{
	public delegate object RemoteHandler(params object[] args);

	internal class _RemoteHandler
	{
		public readonly ulong m_id;
		public readonly DynFunc m_method;

		public Type ReturnType { get; private set; }

		private _RemoteHandler(Delegate deleg)
		{
			ReturnType = deleg.Method.ReturnType;
			m_method = Func.Create(deleg);
			m_id = ExternalsManager.RegisterRemoteFunction(deleg.Method.ReturnType, m_method) | (ulong)_ExternalFunction.Flags.IS_PERSISTENT;
		}
		
		~_RemoteHandler()
		{
			ExternalsManager.UnRegisterRemoteFunction(m_id);
		}

		internal object Invoke(object[] args) => m_method(args);

		public static RemoteHandler Create(Delegate deleg)
		{
			var func = new _RemoteHandler(deleg);
			return new RemoteHandler(func.Invoke);
		}
	}
}
#endif
