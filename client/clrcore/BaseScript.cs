using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public abstract class BaseScript
    {
        private Dictionary<Delegate, Task> CurrentTaskList { get; set; }

        /// <summary>
        /// An event containing callbacks to attempt to schedule on every game tick.
        /// A callback will only be rescheduled once the associated task completes.
        /// </summary>
        protected event Func<Task> Tick;

        protected EventHandlerDictionary EventHandlers { get; private set; }

        public BaseScript()
        {
            EventHandlers = new EventHandlerDictionary();
        }
        
        internal void ScheduleRun()
        {
            var calls = Tick.GetInvocationList();

            foreach (var call in calls)
            {
                ScheduleTick(call);
            }
        }

        internal void ScheduleTick(Delegate call)
        {
            if (!CurrentTaskList.ContainsKey(call))
            {
                CurrentTaskList.Add(call, CitizenTaskScheduler.Factory.StartNew((Func<Task>)call).Unwrap().ContinueWith(a => CurrentTaskList.Remove(call)));
            }
        }
        
        /// <summary>
        /// Returns a task that will delay scheduling of the current interval function by the passed amount of time.
        /// </summary>
        /// <example>
        /// await Delay(500);
        /// </example>
        /// <param name="msecs">The amount of time by which to delay scheduling this interval function.</param>
        /// <returns>An awaitable task.</returns>
        protected Task Delay(int msecs)
        {
            return CitizenTaskScheduler.Factory.FromAsync(BeginDelay, EndDelay, msecs, null);
        }

        private IAsyncResult BeginDelay(int delay, AsyncCallback callback, object state)
        {
            RuntimeManager.AddDelay(delay, callback);

            return new DummyAsyncResult();
        }

        private void EndDelay(IAsyncResult result)
        {
            
        }
    }

    class DummyAsyncResult : IAsyncResult
    {
        public object AsyncState { get { return null; } }

        public System.Threading.WaitHandle AsyncWaitHandle { get { return null; } }

        public bool CompletedSynchronously { get { return false; } }

        public bool IsCompleted { get { return false; } }
    }
}
