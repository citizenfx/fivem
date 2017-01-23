using System;

namespace CitizenFX.Core
{
	interface InternalManagerInterface
	{
		void SetScriptHost(IntPtr host, int instanceId);

		void CreateAssembly(byte[] assemblyData, byte[] symbolData);

		void Tick();

		void TriggerEvent(string eventName, byte[] argsSerialized, string sourceString);

		void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retvalSerialized, out int retvalSize);

		int DuplicateRef(int refIndex);

		void RemoveRef(int refIndex);
	}
}