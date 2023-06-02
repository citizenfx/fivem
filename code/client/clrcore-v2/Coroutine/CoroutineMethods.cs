using System.Collections.Generic;
using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	public partial class Coroutine
	{
		public static Coroutine Completed()
		{
			var coroutine = new Coroutine();
			coroutine.Complete();
			return coroutine;
		}

		public static Coroutine<T> Completed<T>(T result)
		{
			var coroutine = new Coroutine<T>();
			coroutine.Complete(result);
			return coroutine;
		}

		#region Delaying

		/// <summary>
		/// Await to stall execution and continue next frame.
		/// </summary>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Yield()
		{
			var coroutine = new Coroutine();
			Scheduler.Schedule(coroutine.Complete);
			return coroutine;
		}

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given time.
		/// </summary>
		/// <param name="time">Time in milliseconds after we want to continue execution</param>
		/// <remarks>Current time can be retrieved by using <see cref="Scheduler.CurrentTime"/></remarks>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine WaitUntil(TimePoint time)
		{
			var coroutine = new Coroutine();
			Scheduler.Schedule(coroutine.Complete, time);
			return coroutine;
		}

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given time.
		/// </summary>
		/// <param name="time">Time in milliseconds after we want to continue execution</param>
		/// <param name="cancelerToken">Canceling token, can be used to stop early</param>
		/// <remarks>Current time can be retrieved by using <see cref="Scheduler.CurrentTime"/></remarks>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine WaitUntil(TimePoint time, CancelerToken cancelerToken)
		{
			if (!cancelerToken.CancelOrThrowIfRequested())
			{
				var coroutine = new Coroutine();

				void OnCancel()
				{
					Scheduler.Unschedule(OnWaited, time);
					coroutine.Complete();
				}

				void OnWaited()
				{
					coroutine.Complete();
					cancelerToken.OnCancel -= OnCancel;
				}

				Scheduler.Schedule(OnWaited, time);
				cancelerToken.OnCancel += OnCancel;

				return coroutine;
			}

			return Completed();
		}

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given delay.
		/// </summary>
		/// <param name="delay">Delay in milliseconds after we want to continue execution</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Wait(ulong delay) => WaitUntil(Scheduler.CurrentTime + delay);

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given delay.
		/// </summary>
		/// <param name="delay">Delay in milliseconds after we want to continue execution</param>
		/// <param name="cancelerToken">Canceling token, can be used to stop early</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Wait(ulong delay, CancelerToken cancelerToken) => WaitUntil(Scheduler.CurrentTime + delay, cancelerToken);

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given delay.
		/// </summary>
		/// <param name="delay">Delay in milliseconds after we want to continue execution</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Delay(ulong delay) => WaitUntil(Scheduler.CurrentTime + delay);

		/// <summary>
		/// Await to stall execution of the current async method and continue after the given delay.
		/// </summary>
		/// <param name="delay">Delay in milliseconds after we want to continue execution</param>
		/// <param name="cancelerToken">Canceling token, can be used to stop early</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Delay(ulong delay, CancelerToken cancelerToken) => WaitUntil(Scheduler.CurrentTime + delay, cancelerToken);

		#endregion

		#region Scheduling

		/// <summary>
		/// Schedules a delegate to be run at a later time
		/// </summary>
		/// <param name="function">Delegate to run</param>
		/// <param name="delay">Delay until the <paramref name="function"/> is executed</param>
		/// <param name="iterations">Amount of times we want to run the <paramref name="function"/>. <see cref="Repeat.Infinite"/> or <see cref="ulong.MaxValue"/> to infinitely loop.</param>
		/// <param name="cancelerToken">Allows the cancellation of the given <paramref name="function"/> before it got executed</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static async Coroutine Schedule(Func<Coroutine> function, ulong delay, ulong iterations = Repeat.Infinite, CancelerToken cancelerToken = default)
		{
			if (iterations > 0)
			{
				while ((iterations == Repeat.Infinite || iterations-- > 0) && !cancelerToken.CancelOrThrowIfRequested())
				{
					await Delay(delay, cancelerToken);

					if (cancelerToken.CancelOrThrowIfRequested())
						return;

					await function();
				}
			}
		}

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static async Coroutine<T> Schedule<T>(Func<Coroutine<T>> function, ulong delay, ulong iterations = Repeat.Infinite, CancelerToken cancelerToken = default)
		{
			T result = default;

			if (iterations > 0)
			{
				while ((iterations == Repeat.Infinite || iterations-- > 0) && !cancelerToken.CancelOrThrowIfRequested())
				{
					await Delay(delay, cancelerToken);

					if (cancelerToken.CancelOrThrowIfRequested())
						return result;

					result = await function();
				}
			}

			return result;
		}

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static async Coroutine Schedule(Func<Coroutine> function, TimePoint time, CancelerToken cancelerToken = default)
		{
			await WaitUntil(time, cancelerToken);

			if (!cancelerToken.CancelOrThrowIfRequested())
			{
				await function();
			}
		}

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static async Coroutine<T> Schedule<T>(Func<Coroutine<T>> function, TimePoint time, CancelerToken cancelerToken = default)
		{
			await WaitUntil(time, cancelerToken);
			return !cancelerToken.CancelOrThrowIfRequested() ? await function() : default;
		}

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine Schedule(Func<Coroutine> function)
			=> Schedule(function, Scheduler.CurrentTime, default);

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine Schedule(Func<Coroutine> function, ulong delay, CancelerToken cancelerToken)
			=> Schedule(function, Scheduler.CurrentTime + delay, cancelerToken);

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine<T> Schedule<T>(Func<Coroutine<T>> function, ulong delay, CancelerToken cancelerToken)
			=> Schedule(function, Scheduler.CurrentTime + delay, cancelerToken);

#pragma warning disable CS1998 // Async method lacks 'await' operators and will run synchronously
		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine Schedule(Action function)
			=> Schedule(async () => function(), Scheduler.CurrentTime, default);

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine Schedule(Action function, ulong delay, CancelerToken cancelerToken = default)
			=> Schedule(async () => function(), Scheduler.CurrentTime + delay, cancelerToken);

		/// <inheritdoc cref="Schedule(Func{Coroutine}, ulong, ulong, CancelerToken)"/>
		public static Coroutine Schedule(Action function, ulong delay, ulong iterations, CancelerToken cancelerToken = default)
			=> Schedule(async () => function(), delay, iterations, cancelerToken);
#pragma warning restore CS1998 // Async method lacks 'await' operators and will run synchronously

		#endregion

		#region Run

		/// <summary>
		/// Runs given function directly and allows the caller to await it.
		/// </summary>
		/// <param name="function">Function to run</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Run(Func<Coroutine> function) => function();

		/// <inheritdoc cref="Run(Func{Coroutine})"/>
		public static Coroutine Run(Action function)
		{
			function();
			return Completed();
		}

		/// <summary>
		/// Runs given function directly and allows the caller to await it.
		/// </summary>
		/// <param name="function">Function to run</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine Run<TResult>(Func<Coroutine<TResult>> function) => function();

		/// <summary>
		/// Runs given function next frame and allows the caller to await it.
		/// </summary>
		/// <param name="function">Function to run</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine RunNextFrame(Func<Coroutine> function)
		{
			var coroutine = new Coroutine();
			Scheduler.Schedule(async () =>
			{
				await function();
				coroutine.Complete();
			});
			return coroutine;
		}

		/// <inheritdoc cref="RunNextFrame(Func{Coroutine})"/>
		public static Coroutine RunNextFrame(Action function)
		{
			var coroutine = new Coroutine();
			Scheduler.Schedule(() =>
			{
				function();
				coroutine.Complete();
			});
			return coroutine;
		}

		/// <summary>
		/// Runs given function directly and allows the caller to await it.
		/// </summary>
		/// <param name="function">Function to run</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine<T> RunNextFrame<T>(Func<Coroutine<T>> function)
		{
			var coroutine = new Coroutine<T>();
			Scheduler.Schedule(async () =>
			{
				var result = await function();
				coroutine.Complete(result);
			});
			return coroutine;
		}

		#endregion

		#region Multiple awaitment

		/// <summary>
		/// Run all functions and await the first one to complete
		/// </summary>
		/// <remarks>Will run the coroutines directly</remarks>
		/// <param name="coroutines">Coroutines to await</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine WhenAny(params Coroutine[] coroutines)
		{
			var coroutine = new Coroutine();
			var awaitees = new List<Coroutine>();

			void Complete()
			{
				// disable continuation calls for all awaited coroutines
				for (int i = 0; i < awaitees.Count; ++i)
				{
					awaitees[i].ClearContinueWith();
				}

				coroutine.Complete();
			}

			for (int i = 0; i < coroutines.Length; ++i)
			{
				Coroutine active = coroutines[i];
				if (!active.IsCompleted)
				{
					awaitees.Add(active);
					active.ContinueWith(Complete);
				}
				else
				{
					return active;
				}
			}

			return coroutine;
		}

		/// <summary>
		/// Run all functions and await the first one to complete
		/// </summary>
		/// <remarks>Will run the coroutines directly</remarks>
		/// <param name="coroutines">Coroutines to await</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine<T> WhenAny<T>(params Coroutine<T>[] coroutines)
		{
			var coroutine = new Coroutine<T>();
			var awaitees = new List<Coroutine<T>>();

			void Complete(Coroutine awaitee)
			{
				// disable continuation calls for all awaited coroutines
				for (int i = 0; i < awaitees.Count; ++i)
				{
					awaitees[i].m_continuation = null;
				}

				coroutine.Complete((awaitee as Coroutine<T>).Result);
			}

			for (int i = 0; i < coroutines.Length; ++i)
			{
				Coroutine<T> active = coroutines[i];
				if (!active.IsCompleted)
				{
					awaitees.Add(active);
					active.ContinueWith(Complete);
				}
				else
				{
					return active;
				}
			}

			return coroutine;
		}

		/// <summary>
		/// Run all functions and await all to complete
		/// </summary>
		/// <param name="coroutines">Coroutines to await</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine WhenAll(params Coroutine[] coroutines)
		{
			var coroutine = new Coroutine();
			var awaitees = new List<Coroutine>();
			var exceptions = new List<Exception>();

			void OnAwaiteeComplete(Coroutine awaitee)
			{
				if (awaitee.Exception != null)
				{
					exceptions.Add(awaitee.Exception);
				}

				awaitees.Remove(awaitee);
				if (awaitees.Count == 0)
				{
					if (exceptions.Count == 0)
						coroutine.Complete();
					else
						coroutine.Fail(null, new AggregateException(exceptions.ToArray()));
				}
			}

			for (int i = 0; i < coroutines.Length; ++i)
			{
				Coroutine active = coroutines[i];
				if (!active.IsCompleted)
				{
					awaitees.Add(active);
					active.ContinueWith(OnAwaiteeComplete);
				}
			}

			return awaitees.Count > 0 ? coroutine : Completed();
		}

		/// <summary>
		/// Run all functions and await all to complete
		/// </summary>
		/// <param name="coroutines">Coroutines to await</param>
		/// <returns>Awaitable <see cref="Coroutine"/></returns>
		public static Coroutine WhenAll<T>(params Coroutine<T>[] coroutines)
		{
			var coroutine = new Coroutine();
			var awaitees = new List<Coroutine>();
			var exceptions = new List<Exception>();

			void OnAwaiteeComplete(Coroutine awaitee)
			{
				if (awaitee.Exception != null)
				{
					exceptions.Add(awaitee.Exception);
				}

				awaitees.Remove(awaitee);
				if (awaitees.Count == 0)
				{
					if (exceptions.Count == 0)
						coroutine.Complete((awaitee as Coroutine<T>).Result);
					else
						coroutine.Fail(null, new AggregateException(exceptions.ToArray()));
				}
			}

			for (int i = 0; i < coroutines.Length; ++i)
			{
				Coroutine active = coroutines[i];
				if (!active.IsCompleted)
				{
					awaitees.Add(active);
					active.ContinueWith(OnAwaiteeComplete);
				}
			}

			return awaitees.Count > 0 ? coroutine : Completed();
		}

		#endregion
	}
}
