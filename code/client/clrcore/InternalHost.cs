using System;

namespace CitizenFX.Core
{
	class InternalHost : MarshalByRefObject, IInternalHost
	{
		private readonly IScriptHost m_host;

		public InternalHost(IScriptHost host)
		{
			m_host = host;
		}

		public void InvokeNative(ref fxScriptContext context)
		{
			m_host.InvokeNative(context);
		}
	}
}