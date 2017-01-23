using System;

namespace CitizenFX.Core
{
	internal class PushRuntime : IDisposable
	{
		private readonly IScriptRuntime m_runtime;
		private readonly IScriptRuntimeHandler m_runtimeHandler;

		public PushRuntime(IScriptRuntime runtime)
		{
			m_runtimeHandler = InternalManager.CreateInstance<IScriptRuntimeHandler>(new Guid(0xc41e7194, 0x7556, 0x4c02, 0xba, 0x45, 0xa9, 0xc8, 0x4d, 0x18, 0xad, 0x43));
			m_runtimeHandler.PushRuntime(runtime);

			m_runtime = runtime;
		}

		public void Dispose()
		{
			m_runtimeHandler.PopRuntime(m_runtime);
		}
	}
}