using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("4720a986-eaa6-4ecc-a31f-2ce2bbf569f7")]
	[ComImport]
	public interface IScriptRuntimeHandler
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		void PushRuntime(IScriptRuntime runtime);

		[MethodImpl(MethodImplOptions.InternalCall)]
		IScriptRuntime GetCurrentRuntime();

		[MethodImpl(MethodImplOptions.InternalCall)]
		void PopRuntime(IScriptRuntime runtime);
	}
}