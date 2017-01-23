using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
	[System.Runtime.InteropServices.Guid("82ec2441-dbb4-4512-81e9-3a98ce9ffcab")]
	[ComImport]
	public interface fxIStream
	{
		[MethodImpl(MethodImplOptions.InternalCall)]
		int Read(byte[] data, int size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		int Write(byte[] data, int size);

		[MethodImpl(MethodImplOptions.InternalCall)]
		long Seek(long offset, int origin);

		[MethodImpl(MethodImplOptions.InternalCall)]
		long GetLength();
	}
}