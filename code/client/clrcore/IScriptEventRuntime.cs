using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("637140db-24e5-46bf-a8bd-08f2dbac519a")]
	public interface IScriptEventRuntime
	{
		void TriggerEvent([MarshalAs(UnmanagedType.LPStr)] string eventName,
						  [In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] argsSerialized,
						  int serializedSize,
						  [MarshalAs(UnmanagedType.LPStr)] string sourceId);
	}
}