using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("91b203c7-f95a-4902-b463-722d55098366")]
	public interface IScriptTickRuntime
	{
		void Tick();
	}

	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("182CAAF3-E33D-474B-A6AF-33D59FF0E9ED")]
	[ComImport]
	public interface IScriptStackWalkVisitor
	{
		void SubmitStackFrame([In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] byte[] frameBlob, int frameBlobSize);
	}

	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[Guid("567D2FDA-610C-4FA0-AE3E-4F700AE5CE56")]
	public interface IScriptStackWalkingRuntime
	{
		void WalkStack(
			[In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 1)] byte[] boundaryStart,
			[In] int boundaryStartLength,
			[In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 3)] byte[] boundaryEnd,
			[In] int boundaryEndLength,
			[In] IScriptStackWalkVisitor visitor);
	}
}
