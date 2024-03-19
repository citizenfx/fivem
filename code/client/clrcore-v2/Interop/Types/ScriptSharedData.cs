using System.Runtime.InteropServices;
using System.Threading;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Explicit)]
	internal struct ScriptSharedData
	{
		/// <summary>
		/// Next time when our host needs to call in again
		/// </summary>
		[FieldOffset(0)] public ulong m_scheduledTime;

		/// <summary>
		/// Same as <see cref="m_scheduledTime"/> but used in methods like <see cref="Interlocked.CompareExchange(ref long, long, long)" /> who miss a <see cref="ulong"/> overload.
		/// </summary>
		[FieldOffset(0)] public long m_scheduledTimeAsLong;
	};
}
