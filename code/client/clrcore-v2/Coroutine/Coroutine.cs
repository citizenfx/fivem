using System;
using System.Runtime.CompilerServices;
using System.Runtime.ExceptionServices;
using System.Threading;

namespace CitizenFX.Core
{
	public sealed class Promise : Coroutine { }
	public sealed class Promise<T> : Coroutine<T> { }

	public sealed class CoroutineBuilder
	{
		public Coroutine Task { get; } = new Coroutine();

		public static CoroutineBuilder Create() => new CoroutineBuilder();

		public void SetResult() => Task.Complete();

		public void Start<TStateMachine>(ref TStateMachine stateMachine) where TStateMachine : IAsyncStateMachine => stateMachine.MoveNext();

		public void SetException(Exception exception) => Task.SetException(exception);

		public void AwaitOnCompleted<TAwaiter, TStateMachine>(ref TAwaiter awaiter, ref TStateMachine stateMachine)
			where TAwaiter : INotifyCompletion where TStateMachine : IAsyncStateMachine
		{
			TStateMachine sMachine = stateMachine;
			awaiter.OnCompleted(() =>
			{
				if (Thread.CurrentThread == Scheduler.MainThread)
					sMachine.MoveNext();
				else
					Scheduler.Schedule(sMachine.MoveNext); // Schedule Coroutines back to the main thread
			});
		}

		public void AwaitUnsafeOnCompleted<TAwaiter, TStateMachine>(ref TAwaiter awaiter, ref TStateMachine stateMachine)
			where TAwaiter : ICriticalNotifyCompletion where TStateMachine : IAsyncStateMachine
		{
			TStateMachine sMachine = stateMachine;
			awaiter.UnsafeOnCompleted(() =>
			{
				if (Thread.CurrentThread == Scheduler.MainThread)
					sMachine.MoveNext();
				else
					Scheduler.Schedule(sMachine.MoveNext); // Schedule Coroutines back to the main thread
			});
		}

		public void SetStateMachine(IAsyncStateMachine stateMachine)
		{
			Debug.WriteLine("SetStateMachine");
		}
	}

	public sealed class CoroutineBuilder<T>
	{
		public Coroutine<T> Task { get; } = new Coroutine<T>();

		public static CoroutineBuilder<T> Create() => new CoroutineBuilder<T>();

		public void SetResult(T value) => Task.Complete(value);

		public void SetException(Exception exception) => Task.SetException(exception);

		public void Start<TStateMachine>(ref TStateMachine stateMachine) where TStateMachine : IAsyncStateMachine => stateMachine.MoveNext();

		public void AwaitOnCompleted<TAwaiter, TStateMachine>(ref TAwaiter awaiter, ref TStateMachine stateMachine)
			where TAwaiter : INotifyCompletion where TStateMachine : IAsyncStateMachine
		{
			TStateMachine sMachine = stateMachine;
			awaiter.OnCompleted(() =>
			{
				if (Thread.CurrentThread == Scheduler.MainThread)
					sMachine.MoveNext();
				else
					Scheduler.Schedule(sMachine.MoveNext); // Schedule Coroutines back to the main thread
			});
		}

		public void AwaitUnsafeOnCompleted<TAwaiter, TStateMachine>(ref TAwaiter awaiter, ref TStateMachine stateMachine)
			where TAwaiter : ICriticalNotifyCompletion where TStateMachine : IAsyncStateMachine
		{
			TStateMachine sMachine = stateMachine;
			awaiter.UnsafeOnCompleted(() =>
			{
				if (Thread.CurrentThread == Scheduler.MainThread)
					sMachine.MoveNext();
				else
					Scheduler.Schedule(sMachine.MoveNext); // Schedule Coroutines back to the main thread
			});
		}

		public void SetStateMachine(IAsyncStateMachine stateMachine)
		{
			Debug.WriteLine("SetStateMachine");
		}
	}

	[AsyncMethodBuilder(typeof(CoroutineBuilder<>))]
	public class Coroutine<T> : Coroutine
	{
		private T m_result;

		public T Result
		{
			get
			{
				m_exception?.Throw();
				return m_result;
			}
		}

		internal Coroutine() { }

		public new CoroutineAwaiter<T> GetAwaiter() => new CoroutineAwaiter<T>(this);

		public new T GetResult() => Result;

		protected override object GetResultInternal() => m_result;

		public void Complete(T value) => CompleteInternal(State.Completed, value);
		public override void Complete(object value) => CompleteInternal(State.Completed, (T)value);
		public override void Complete(object value, Exception ex) => CompleteInternal(State.Completed, (T)value, ex);

		public void Cancel(T value) => CompleteInternal(State.Canceled, value);
		public override void Cancel(object value) => CompleteInternal(State.Canceled, (T)value);
		public override void Cancel(object value, Exception ex) => CompleteInternal(State.Canceled, (T)value, ex);

		public void Fail(T value) => CompleteInternal(State.Failed, value);
		public override void Fail(object value) => CompleteInternal(State.Failed, (T)value);
		public override void Fail(object value, Exception ex) => CompleteInternal(State.Failed, (T)value, ex);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private void CompleteInternal(State completionState, T value)
		{
			m_result = value;
			base.CompleteInternal(completionState);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private void CompleteInternal(State completionState, T value, Exception exception)
		{
			m_result = value;
			base.CompleteInternal(completionState, exception);
		}

		public static Coroutine<T> Completed(T completeWithValue)
		{
			var coroutine = new Coroutine<T>();
			coroutine.Complete(completeWithValue);
			return coroutine;
		}
	}

	[AsyncMethodBuilder(typeof(CoroutineBuilder))]
	public partial class Coroutine
	{
		public enum State : uint
		{
			// 0x000F
			Idle = 0,
			Active = 1u << 1,

			// 0x00F0
			// reserved

			// 0x0F00
			// reserved

			// 0xF000
			Completed = 1u << 24,
			Succesful = Completed, // no other bits set
			Failed = Completed | (1 << 25),
			Canceled = Completed | (1u << 26),

			MaskActive = 0x000F,
			MaskCompletion = 0xF000,
		}

		private Action<Coroutine> m_continuation;

		protected ExceptionDispatchInfo m_exception;

		protected State m_state;

		public Exception Exception
		{
			get => m_exception?.SourceException;
			set => m_exception = value != null ? ExceptionDispatchInfo.Capture(value) : null;
		}

		public bool IsCompleted => (m_state & State.Completed) != 0;
		public bool IsSuccessful => (m_state & State.MaskCompletion) == State.Succesful;
		public bool IsFailed => (m_state & State.MaskCompletion) == State.Failed;
		public bool IsCanceled => (m_state & State.MaskCompletion) == State.Canceled;

		internal Coroutine() { }

		public object GetResult()
		{
			m_exception?.Throw();
			return GetResultInternal();
		}

		internal object GetResultNonThrowing() => GetResultInternal();

		protected virtual object GetResultInternal() => null;

		internal void Complete() => CompleteInternal(State.Completed);

		public virtual void Complete(object value = null) => CompleteInternal(State.Completed);
		public virtual void Complete(object value, Exception exception) => CompleteInternal(State.Completed, exception);

		public virtual void Cancel(object value = null) => CompleteInternal(State.Canceled);
		public virtual void Cancel(object value, Exception exception) => CompleteInternal(State.Canceled, exception);

		public virtual void Fail(object value = null) => CompleteInternal(State.Failed);
		public virtual void Fail(object value, Exception exception) => CompleteInternal(State.Failed, exception);

		internal void SetException(Exception exception) => CompleteInternal(State.Failed, exception);

		protected void CompleteInternal(State completionState)
		{
			if (!IsCompleted)
			{
				m_state |= completionState;
				m_continuation?.Invoke(this);
			}
		}

		protected void CompleteInternal(State completionState, Exception exception)
		{
			Exception = exception;
			CompleteInternal(completionState);
		}

		/// <summary>
		/// Adds an action that will be called when this coroutine is done
		/// </summary>
		/// <param name="action">Action to call when coroutine completes</param>
		public void ContinueWith(Action<Coroutine> action) => m_continuation += action;

		/// <summary>
		/// Adds an action that will be called when this coroutine is done
		/// </summary>
		/// <param name="action">Action to call when coroutine completes</param>
		public void ContinueWith(Action action) => ContinueWith(_ => action());

		/// <summary>
		/// Removes all continued actions, e.g.: <see cref="ContinueWith(Action{Coroutine})"/> and sets one that writes any exception thrown instead
		/// </summary>
		/// <remarks>Unsafe. Should only be used when you know the exact state of this coroutine</remarks>
		internal void ClearContinueWith()
		{
			m_continuation = coroutine =>
			{
				if (coroutine.Exception != null)
				{
					Debug.WriteLine(coroutine.Exception);
				}
			};
		}

		public CoroutineAwaiter GetAwaiter() => new CoroutineAwaiter(this);
	}

	public interface ICoroutineAwaiter {}

	public readonly struct CoroutineAwaiter<T> : ICriticalNotifyCompletion, ICoroutineAwaiter
	{
		readonly Coroutine<T> m_coroutine;

		public bool IsCompleted => m_coroutine.IsCompleted;

		internal CoroutineAwaiter(Coroutine<T> coroutine) => m_coroutine = coroutine;

		public T GetResult() => m_coroutine.GetResult();

		public void SetResult(T result) => m_coroutine.Complete(result);

		public void SetException(Exception exception) => m_coroutine.SetException(exception);

		public void OnCompleted(Action continuation) => m_coroutine.ContinueWith(continuation);

		public void UnsafeOnCompleted(Action continuation) => m_coroutine.ContinueWith(continuation);
	}

	public readonly struct CoroutineAwaiter : ICriticalNotifyCompletion, ICoroutineAwaiter
	{
		readonly Coroutine m_coroutine;

		public bool IsCompleted => m_coroutine.IsCompleted;

		internal CoroutineAwaiter(Coroutine coroutine) => m_coroutine = coroutine;

		public void GetResult() => m_coroutine.GetResult();

		public void SetResult() => m_coroutine.Complete();

		public void SetException(Exception exception) => m_coroutine.SetException(exception);

		public void OnCompleted(Action continuation) => m_coroutine.ContinueWith(continuation);

		public void UnsafeOnCompleted(Action continuation) => m_coroutine.ContinueWith(continuation);
	}
}
