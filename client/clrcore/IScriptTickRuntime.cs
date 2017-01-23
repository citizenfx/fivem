using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("91b203c7-f95a-4902-b463-722d55098366")]
	public interface IScriptTickRuntime
	{
		void Tick();
	}
}