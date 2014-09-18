using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    class CitizenTaskScheduler : TaskScheduler
    {
        private List<Task> m_runningTasks = new List<Task>();

        protected CitizenTaskScheduler()
        {
            
        }

        [SecurityCritical]
        protected override void QueueTask(Task task)
        {
            m_runningTasks.Add(task);
        }

        [SecurityCritical]
        protected override bool TryExecuteTaskInline(Task task, bool taskWasPreviouslyQueued)
        {
            if (!taskWasPreviouslyQueued)
            {
                return TryExecuteTask(task);
            }

            return false;
        }

        [SecurityCritical]
        protected override IEnumerable<Task> GetScheduledTasks()
        {
            return m_runningTasks;
        }

        public override int MaximumConcurrencyLevel
        {
            get
            {
                return 1;
            }
        }

        [SecuritySafeCritical]
        public void Tick()
        {
            var tasks = m_runningTasks.ToArray();

            foreach (var task in tasks)
            {
                TryExecuteTask(task);

                if (task.IsCompleted || task.IsFaulted || task.IsCanceled)
                {
                    m_runningTasks.Remove(task);
                }
            }
        }

        public static void Create()
        {
            Instance = new CitizenTaskScheduler();

            Factory = new TaskFactory(Instance);
        }

        public static TaskFactory Factory { get; private set; }

        public static CitizenTaskScheduler Instance { get; private set; }
    }
}
