using System;

namespace CitizenFX.Core
{
	internal class PushRuntime : IDisposable
	{
		private readonly IScriptRuntime m_runtime;
		private static IScriptRuntimeHandler ms_runtimeHandler;

		public PushRuntime(IScriptRuntime runtime)
		{
			EnsureRuntimeHandler();

			ms_runtimeHandler.PushRuntime(runtime);

			m_runtime = runtime;
		}

		public void Dispose()
		{
			ms_runtimeHandler.PopRuntime(m_runtime);
		}

		private static void EnsureRuntimeHandler()
		{
			if (ms_runtimeHandler == null)
			{
				ms_runtimeHandler = InternalManager.CreateInstance<IScriptRuntimeHandler>(new Guid(0xc41e7194, 0x7556, 0x4c02, 0xba, 0x45, 0xa9, 0xc8, 0x4d, 0x18, 0xad, 0x43));
			}
		}
	}
}