using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("567634c6-3bdd-4d0e-af39-7472aed479b7")]
	public interface IScriptFileHandlingRuntime
	{
		[PreserveSig]
		int HandlesFile([MarshalAs(UnmanagedType.LPStr)] string filename);

		void LoadFile([MarshalAs(UnmanagedType.LPStr)]string scriptFile);
	}
}