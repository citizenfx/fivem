using System;

namespace CitizenFX.Core
{
	interface InternalManagerInterface
	{
		void SetResourceName(string resourceName);

		void CreateTaskScheduler();

		void Destroy();

		void SetScriptHost(IScriptHost host, int instanceId);

		void CreateAssembly(string name, byte[] assemblyData, byte[] symbolData);

		void LoadAssembly(string name);

		void Tick();

		void TriggerEvent(string eventName, byte[] argsSerialized, string sourceString);

		void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retvalSerialized, out int retvalSize);

		int DuplicateRef(int refIndex);

		void RemoveRef(int refIndex);

		ulong GetMemoryUsage();

		byte[] WalkStack(byte[] boundaryStart, byte[] boundaryEnd);
	}
}
