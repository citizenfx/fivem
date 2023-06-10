
namespace CitizenFX.Core
{
	/// <summary>
	/// Absolute time for scheduling on fixed points in time
	/// </summary>
	public readonly struct TimePoint
	{
		private readonly ulong m_time;
		
		public TimePoint(ulong timeInMilliseconds) => m_time = timeInMilliseconds;
		
		public static TimePoint operator+ (TimePoint timePoint, ulong timeInMilliseconds) => new TimePoint(timePoint.m_time + timeInMilliseconds);
		public static TimePoint operator -(TimePoint timePoint, ulong timeInMilliseconds) => new TimePoint(timePoint.m_time - timeInMilliseconds);
		public static TimePoint operator *(TimePoint timePoint, ulong timeInMilliseconds) => new TimePoint(timePoint.m_time * timeInMilliseconds);
		public static TimePoint operator /(TimePoint timePoint, ulong timeInMilliseconds) => new TimePoint(timePoint.m_time / timeInMilliseconds);

		public static implicit operator ulong(TimePoint timePoint) => timePoint.m_time;
		public static explicit operator TimePoint(ulong timeInMilliseconds) => new TimePoint(timeInMilliseconds);

		public override string ToString() => m_time.ToString();
	}

	public struct Repeat
	{
		public const ulong Infinite = ulong.MaxValue;
	}
}
