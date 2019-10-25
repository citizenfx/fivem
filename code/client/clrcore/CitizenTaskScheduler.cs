using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Security;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	class CitizenSynchronizationContext : SynchronizationContext
	{
		private static readonly List<Action> m_scheduledTasks = new List<Action>();

		public override void Post(SendOrPostCallback d, object state)
		{
			lock (m_scheduledTasks)
			{
				m_scheduledTasks.Add(() => d(state));
			}
		}

		public static void Tick()
		{
			Action[] tasks;

			lock (m_scheduledTasks)
			{
				tasks = m_scheduledTasks.ToArray();
				m_scheduledTasks.Clear();
			}

			foreach (var task in tasks)
			{
				try
				{
					task();
				}
				catch (Exception e)
				{
					Debug.WriteLine($"Exception during executing Post callback: {e}");
				}
			}
		}

		public override SynchronizationContext CreateCopy()
		{
			return this;
		}
	}

    class CitizenTaskScheduler : TaskScheduler
    {
		private static readonly object m_inTickTasksLock = new object();
		private Dictionary<int, Task> m_inTickTasks;

        private readonly Dictionary<int, Task> m_runningTasks = new Dictionary<int, Task>();

        protected CitizenTaskScheduler()
        {
            
        }

        [SecurityCritical]
        protected override void QueueTask(Task task)
        {
			if (m_inTickTasks != null)
			{
				lock (m_inTickTasksLock)
				{
					if (m_inTickTasks != null)
						m_inTickTasks[task.Id] = task;
				}
			}

			lock (m_runningTasks)
			{
				m_runningTasks[task.Id] = task;
			}
        }

        [SecurityCritical]
        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
			if (!taskWasPreviouslyQueued)
            {
				//using (var scope = new ProfilerScope(() => GetTaskName(task)))
				{
					return TryExecuteTask(task);
				}
            }

            return false;
        }

        [SecurityCritical]
        protected override IEnumerable<Task> GetScheduledTasks()
        {
			lock (m_runningTasks)
			{
				return m_runningTasks.Select(a => a.Value).ToArray();
			}
        }

        public override int MaximumConcurrencyLevel => 1;

	    public void Tick()
        {
			Task[] tasks;

			lock (m_runningTasks)
			{
				tasks = m_runningTasks.Values.ToArray();
			}

			// ticks should be reentrant (Tick might invoke TriggerEvent, e.g.)
			Dictionary<int, Task> lastInTickTasks;

			lock (m_inTickTasksLock)
			{
				lastInTickTasks = m_inTickTasks;

				m_inTickTasks = new Dictionary<int, Task>();
			}

			do
			{
				using (var scope = new ProfilerScope(() => "task iteration"))
				{
					foreach (var task in tasks)
					{
						InvokeTryExecuteTask(task);

						if (task.Exception != null)
						{
							Debug.WriteLine("Exception thrown by a task: {0}", task.Exception.ToString());
						}

						if (task.IsCompleted || task.IsFaulted || task.IsCanceled)
						{
							lock (m_runningTasks)
							{
								m_runningTasks.Remove(task.Id);
							}
						}
					}

					lock (m_inTickTasksLock)
					{
						tasks = m_inTickTasks.Values.ToArray();
						m_inTickTasks.Clear();
					}
				}
			} while (tasks.Length != 0);

			lock (m_inTickTasksLock)
			{
				m_inTickTasks = lastInTickTasks;
			}
        }

        [SecuritySafeCritical]
        private bool InvokeTryExecuteTask(Task task)
        {
			//using (var scope = new ProfilerScope(() => GetTaskName(task)))
			{
				return TryExecuteTask(task);
			}
        }

		private static FieldInfo ms_taskFieldInfo = typeof(Task).GetField("m_action", BindingFlags.Instance | BindingFlags.NonPublic);

		private string GetTaskName(Task task)
		{
			var action = ms_taskFieldInfo.GetValue(task);

			if (action is Delegate deleg)
			{
				return $"{deleg.Method.DeclaringType.Name} -> task {deleg.Method.Name}";
			}

			return action?.ToString() ?? task.ToString();
		}

		[SecuritySafeCritical]
        public static void Create()
        {
            Instance = new CitizenTaskScheduler();

            Factory = new TaskFactory(Instance);

			TaskScheduler.UnobservedTaskException += TaskScheduler_UnobservedTaskException;
		}

		[SecuritySafeCritical]
		public static void MakeDefault()
		{
			var field = typeof(TaskScheduler).GetField("s_defaultTaskScheduler", BindingFlags.Static | BindingFlags.NonPublic);
			field.SetValue(null, Instance);

			field = typeof(Task).GetField("<Factory>k__BackingField", BindingFlags.Static | BindingFlags.NonPublic);

			if (field == null)
			{
				field = typeof(Task).GetField("s_factory", BindingFlags.Static | BindingFlags.NonPublic);
			}

			field.SetValue(null, Factory);
		}

		private static void TaskScheduler_UnobservedTaskException(object sender, UnobservedTaskExceptionEventArgs e)
		{
			Debug.WriteLine($"Unhandled task exception: {e.Exception.InnerExceptions.Aggregate("", (a, b) => $"{a}\n{b}")}");

			e.SetObserved();
		}

		public static TaskFactory Factory { get; private set; }

        public static CitizenTaskScheduler Instance { get; private set; }
    }
}
