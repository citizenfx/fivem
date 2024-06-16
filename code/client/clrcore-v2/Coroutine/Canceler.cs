using System;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public class Canceler
	{
		public bool IsCanceled { get; private set; } = false;
		public bool ThrowOnCancelation { get; set; } = false;

		public event Action OnCancel;

		public CancelerToken Token => new CancelerToken(this);

		public Canceler() { }

		public void Cancel()
		{
			IsCanceled = true;
			OnCancel?.Invoke();
			OnCancel = null;
		}

		public static implicit operator CancelerToken(Canceler canceler) => canceler.Token;
	}

	public readonly struct CancelerToken
	{
		private readonly Canceler m_canceler;

		public bool IsCanceled => m_canceler?.IsCanceled == true;
		public bool ThrowOnCancelation => m_canceler?.ThrowOnCancelation == true;

		public event Action OnCancel
		{
			add { if (m_canceler != null) m_canceler.OnCancel += value; }
			remove { if (m_canceler != null) m_canceler.OnCancel -= value; }
		}

		public CancelerToken(Canceler canceler) => m_canceler = canceler;

		/// <summary>
		/// Combination of <see cref="IsCanceled"/> and <see cref="ThrowOnCancelation"/> for throwing support
		/// </summary>
		/// <returns><see langword="true"/> if we need to stop execution of the coroutine otherwise <see langword="false"/></returns>
		/// <exception cref="CoroutineCanceledException"></exception>
		internal bool CancelOrThrowIfRequested()
		{
			if (m_canceler?.IsCanceled == true)
			{
				if (m_canceler.ThrowOnCancelation)
				{
					throw new CoroutineCanceledException($"Coroutine execution was canceled by a {nameof(Canceler)}");
				}

				return true;
			}

			return false;
		}

		public void AddOnCancelAction(Action action) => m_canceler.OnCancel += action;
	}

	public class CoroutineCanceledException : Exception
	{
		public CoroutineCanceledException() { }
		public CoroutineCanceledException(string message) : base(message) { }
	}
}
