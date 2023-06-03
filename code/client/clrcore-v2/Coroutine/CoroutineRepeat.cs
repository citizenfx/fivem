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
		public bool m_stopOnException;

		private Status m_status = Status.Stopped;

		public CoroutineRepeat(Func<Coroutine> coroutine, bool stopOnException)
		{
			m_coroutine = coroutine;
			m_stopOnException = stopOnException;
		}

		public void Schedule()
		{
			Status curStatus = m_status;
			m_status = Status.Active;

			if (curStatus == Status.Stopped)
			{
				Scheduler.Schedule(Execute);
			}
		}

		private async void Execute()
		{
			while (m_status == Status.Active)
			{
				try
				{
					await m_coroutine();
				}
				catch (Exception ex)
				{
					Debug.WriteLine(ex);

					if (m_stopOnException)
					{
						break;
					}
				}

				await Coroutine.Yield();
			}

			m_status = Status.Stopped;
		}

		public void Stop() => m_status = Status.Stopping;
	}
}
