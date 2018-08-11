using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("D98A35CF-D6EE-4B51-A1C3-99B70F4EC1E6")]
	public interface IScriptMemInfoRuntime
	{
		void RequestMemoryUsage();

		void GetMemoryUsage([Out] out ulong memoryUsage);
	}
}
