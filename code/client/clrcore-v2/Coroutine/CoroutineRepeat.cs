using System;

namespace CitizenFX.Core
{	internal class CoroutineRepeat
	{
		public Func<Coroutine> m_coroutine;

		public bool IsRepeating { get; set; } = true;

		public CoroutineRepeat(Func<Coroutine> coroutine)
		{
			m_coroutine = coroutine;
		}

		public void Schedule()
		{
			IsRepeating = true;

			// Create a repeating action
			Action action = null;
			action = () =>
			{
				var result = m_coroutine();
				if (result != null && !result.GetAwaiter().IsCompleted)
				{
					result.GetAwaiter().OnCompleted(() =>
					{
						if (IsRepeating)
						{
							Scheduler.Schedule(action);
						}
					});
				}
				else if (IsRepeating)
				{
					Scheduler.Schedule(action);
				}
			};

			Scheduler.Schedule(action);
		}

		public void Stop() => IsRepeating = false;
	}
}
