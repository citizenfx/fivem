using System;
using System.Runtime.Remoting;
using System.Security;

namespace CitizenFX.Core
{
	class InternalHost : MarshalByRefObject, IInternalHost
	{
		private readonly IScriptHost m_host;

		public InternalHost(IScriptHost host)
		{
			m_host = host;
		}

		[SecuritySafeCritical]
		public void InvokeNative(ref fxScriptContext context)
		{
			InvokeNativeInternal(ref context);
		}

		[SecurityCritical]
		private void InvokeNativeInternal(ref fxScriptContext context)
		{
			unsafe
			{
				fixed (fxScriptContext* cxt = &context)
				{
					m_host.InvokeNative(new IntPtr(cxt));
				}
			}
		}

		[SecurityCritical]
		public override object InitializeLifetimeService()
		{
			return null;
		}
	}
}
