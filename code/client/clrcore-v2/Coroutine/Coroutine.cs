using System;
using System.Threading;
using System.Runtime.CompilerServices;

namespace CitizenFX.Core
{
	public sealed class Promise : Coroutine { }
	public sealed class Promise<T> : Coroutine<T> { }

	public sealed class CoroutineBuilder
	{
		public Coroutine Task { get; } = new Coroutine();

		public static CoroutineBuilder Create() => new CoroutineBuilder();

		public void SetResult() => Task.GetAwaiter().SetResult();

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

		public void SetResult(T value) => Task.GetAwaiter().SetResult(value);

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
		public T Result { get; private set; }

		internal Coroutine() { }

		public new CoroutineAwaiter<T> GetAwaiter() => new CoroutineAwaiter<T>(this);

		public new T GetResult() => Result;

		protected override object GetResultInternal() => Result;

		public void Complete(T value)
		{
			Result = value;
			CompleteInternal();
		}

		public override void Complete(object value)
		{
			Result = (T)value;
			CompleteInternal();
		}

		public static Coroutine<T> Completed(T completeWithValue)
		{
			var coroutine = new Coroutine<T>();
			coroutine.Complete(completeWithValue);
			return coroutine;
		}
	}

	[AsyncMethodBuilder(typeof(CoroutineBuilder))]
	public class Coroutine
	{
		private Action continuation;

		public Exception Exception { get; set; }

		public bool IsCompleted { get; private set; }

		internal Coroutine() { }

		public object GetResult() => GetResultInternal();

		protected virtual object GetResultInternal() => null;

		public virtual void Complete(object value = null) => CompleteInternal();

		protected void CompleteInternal()
		{
			if (!IsCompleted)
			{
				IsCompleted = true;
				continuation?.Invoke();
			}
		}

		public void SetException(Exception exception)
		{
			Exception = exception;
			CompleteInternal();
		}

		public void ContinueWith(Action action)
		{
			continuation = continuation is null ? action
				: new Action(() => { continuation(); action(); });
		}

		public CoroutineAwaiter GetAwaiter() => new CoroutineAwaiter(this);

		public static Coroutine Completed()
		{
			var coroutine = new Coroutine();
			coroutine.Complete();
			return coroutine;
		}

		public static Coroutine Yield()
		{
			var coroutine = new Coroutine();
			Scheduler.Schedule(() => coroutine.GetAwaiter().SetResult());
			return coroutine;
		}

		internal static Coroutine WaitUntil(uint time)
		{
			var coroutine = new Coroutine();
			Action action = () => coroutine.GetAwaiter().SetResult();

			if (time <= Scheduler.TimeNow) // e.g.: Coroutine.Wait(0u) or Coroutine.Delay(0u)
				Scheduler.Schedule(action);
			else
				Scheduler.Schedule(action, time);

			return coroutine;
		}

		internal static Coroutine Wait(uint delay) => WaitUntil(Scheduler.TimeNow + delay);

		internal static Coroutine Delay(uint delay) => WaitUntil(Scheduler.TimeNow + delay);
	}

	public interface ICoroutineAwaiter {}

	public struct CoroutineAwaiter<T> : ICriticalNotifyCompletion, ICoroutineAwaiter
	{
		Coroutine<T> coroutine;

		public bool IsCompleted => coroutine.IsCompleted;

		internal CoroutineAwaiter(Coroutine<T> coroutine) => this.coroutine = coroutine;

		public T GetResult() => coroutine.GetResult();

		public void SetResult(T result) => coroutine.Complete(result);

		public void SetException(Exception exception) => coroutine.SetException(exception);

		public void OnCompleted(Action continuation) => coroutine.ContinueWith(continuation);

		public void UnsafeOnCompleted(Action continuation) => coroutine.ContinueWith(continuation);
	}

	public struct CoroutineAwaiter : ICriticalNotifyCompletion, ICoroutineAwaiter
	{
		Coroutine coroutine;

		public bool IsCompleted => coroutine.IsCompleted;

		internal CoroutineAwaiter(Coroutine coroutine) => this.coroutine = coroutine;

		public void GetResult() { }

		public void SetResult() => coroutine.Complete();

		public void SetException(Exception exception) => coroutine.SetException(exception);

		public void OnCompleted(Action continuation) => coroutine.ContinueWith(continuation);

		public void UnsafeOnCompleted(Action continuation) => coroutine.ContinueWith(continuation);
	}
}
