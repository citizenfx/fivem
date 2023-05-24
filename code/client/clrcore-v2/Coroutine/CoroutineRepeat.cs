using System;

namespace CitizenFX.Core
{	internal class CoroutineRepeat
	{
		enum Status
		{
			Stopped,
			Active,
			Stopping,
		}

		public Func<Coroutine> m_coroutine;

		private Status m_status = Status.Stopped;

		public CoroutineRepeat(Func<Coroutine> coroutine)
		{
			m_coroutine = coroutine;
		}

		public void Schedule()
		{
			Status curStatus = m_status;
			m_status = Status.Active;

			if (curStatus == Status.Stopped)
			{
				Scheduler.Schedule(Invoke);
			}
		}

		private void Invoke()
		{
			var result = m_coroutine();
			if (result?.GetAwaiter().IsCompleted == false)
			{
				result.GetAwaiter().OnCompleted(Repeat);
			}
			else
			{
				Repeat();
			}
		}

		private void Repeat()
		{			
			if (m_status == Status.Active)
			{
				Scheduler.Schedule(Invoke);
			}
			else
			{
				m_status = Status.Stopped;
			}
		}

		public void Stop() => m_status = Status.Stopping;
	}
}
