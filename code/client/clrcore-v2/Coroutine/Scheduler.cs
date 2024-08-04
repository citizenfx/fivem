using System;
using System.Collections.Generic;
using System.Threading;

namespace CitizenFX.Core
{
    public class Scheduler
    {
        private HashedBinaryHeap coroutineHeap;
        private readonly object lockObject = new object();
        private TimePoint currentTime;
        private bool isRunning;
        private Thread mainThread;
        private ThreadPool threadPool;
        private int threadPoolSize;

        public Scheduler(int initialCapacity = 16)
        {
            coroutineHeap = new HashedBinaryHeap(initialCapacity);
            currentTime = new TimePoint(0);
            mainThread = Thread.CurrentThread;
            threadPoolSize = Environment.ProcessorCount;
            threadPool = new ThreadPool(threadPoolSize);
        }

        public void Schedule(Coroutine coroutine)
        {
            lock (lockObject)
            {
                coroutineHeap.Enqueue(coroutine);
            }
        }

        public void Unschedule(Coroutine coroutine)
        {
            lock (lockObject)
            {
                coroutineHeap.Remove(coroutine);
            }
        }

        public void Run()
        {
            isRunning = true;

            while (isRunning)
            {
                TimePoint nextTime = GetNextTime();

                if (nextTime != null)
                {
                    currentTime = nextTime;

                    List<Coroutine> coroutinesToRun = GetCoroutinesToRun();

                    AdjustThreadPoolSize();

                    foreach (Coroutine coroutine in coroutinesToRun)
                    {
                        if (coroutine.CancelToken.CancelOrThrowIfRequested())
                        {
                            // Handle cancellation
                            Unschedule(coroutine);
                            // Perform any necessary cleanup
                            continue;
                        }

                        threadPool.QueueUserWorkItem(RunCoroutine, coroutine);
                    }
                }
                else
                {
                    isRunning = false;
                }
            }
        }

        private TimePoint GetNextTime()
        {
            lock (lockObject)
            {
                if (coroutineHeap.Count > 0)
                {
                    return coroutineHeap.Peek().Time;
                }
                else
                {
                    return new TimePoint(0); // Return a default TimePoint
                }
            }
        }

        private List<Coroutine> GetCoroutinesToRun()
        {
            lock (lockObject)
            {
                List<Coroutine> coroutinesToRun = new List<Coroutine>();

                while (coroutineHeap.Count > 0 && coroutineHeap.Peek().Time.m_time <= currentTime.m_time)
                {
                    Coroutine coroutine = coroutineHeap.Dequeue();
                    coroutinesToRun.Add(coroutine);
                }

                return coroutinesToRun;
            }
        }

        private void RunCoroutine(object state)
        {
            Coroutine coroutine = (Coroutine)state;
            coroutine.GetAwaiter().GetResult();
        }

        private void AdjustThreadPoolSize()
        {
            int currentCount = coroutineHeap.Count;
            int currentThreadPoolSize = threadPool.GetMaxThreads();

            if (currentCount > currentThreadPoolSize * 0.75)
            {
                int newThreadPoolSize = Math.Min(Environment.ProcessorCount * 2, currentThreadPoolSize * 2);
                threadPool = new ThreadPool(newThreadPoolSize);
            }
            else if (currentCount < currentThreadPoolSize * 0.25)
            {
                int newThreadPoolSize = Math.Max(Environment.ProcessorCount, currentThreadPoolSize / 2);
                threadPool = new ThreadPool(newThreadPoolSize);
            }
        }
    }
}
