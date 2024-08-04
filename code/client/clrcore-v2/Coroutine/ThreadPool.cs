using System;
using System.Threading;

namespace CitizenFX.Core
{
    public class ThreadPool
    {
        private readonly SemaphoreSlim semaphore;
        private readonly int maxThreads;
        private readonly Thread[] threads;

        public ThreadPool(int maxThreads)
        {
            this.maxThreads = maxThreads;
            semaphore = new SemaphoreSlim(maxThreads, maxThreads);
            threads = new Thread[maxThreads];

            for (int i = 0; i < maxThreads; i++)
            {
                threads[i] = new Thread(ThreadProc);
                threads[i].Start();
            }
        }

        public void QueueUserWorkItem(WaitCallback callback, object state)
        {
            semaphore.Wait();

            Thread thread = GetAvailableThread();
            thread.Start(new ThreadState(callback, state));
        }

        private Thread GetAvailableThread()
        {
            foreach (Thread thread in threads)
            {
                if (thread.ThreadState == ThreadState.WaitSleepJoin)
                {
                    return thread;
                }
            }

            throw new InvalidOperationException("No available threads in the thread pool.");
        }

        private void ThreadProc()
        {
            while (true)
            {
                ThreadState state = (ThreadState)Thread.GetData(Thread.GetNamedDataSlot("state"));

                if (state != null)
                {
                    try
                    {
                        state.Callback(state.State);
                    }
                    catch (Exception ex)
                    {
                        // Handle exception
                    }
                    finally
                    {
                        Thread.SetData(Thread.GetNamedDataSlot("state"), null);
                        semaphore.Release();
                    }
                }
                else
                {
                    Thread.Sleep(1); // Sleep for 1 millisecond
                }
            }
        }

        private class ThreadState
        {
            public WaitCallback Callback { get; set; }
            public object State { get; set; }

            public ThreadState(WaitCallback callback, object state)
            {
                Callback = callback;
                State = state;
            }
        }

        public int GetMaxThreads()
        {
            return maxThreads;
        }
    }
}
