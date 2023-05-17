using System;
using System.Threading;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public class Scheduler
	{
		/// <summary>
		/// Current time of this scheduler
		/// </summary>
		public static ulong CurrentTime { get; internal set; }

		/// <summary>
		/// Thread onto which all coroutines will run and be put back to
		/// </summary>
		internal static Thread MainThread { get; private set; }

		/// <summary>
		/// Current time of the scheduler, set per frame.
		/// </summary>
		private static LinkedList<Tuple<ulong, Action>> s_queue = new LinkedList<Tuple<ulong, Action>>();

		/// <summary>
		/// Double buffered array for non-ordered scheduling
		/// </summary>
		private static List<Action> s_nextFrame = new List<Action>();
		private static List<Action> s_nextFrameProcessing = new List<Action>();

		internal static void Initialize()
		{
			MainThread = Thread.CurrentThread;
		}

		/// <summary>
		/// Schedule an action to be run on the next frame
		/// </summary>
		/// <param name="coroutine">Action to execute</param>
		public static void Schedule(Action coroutine)
		{
			if (coroutine != null)
			{
				lock (s_nextFrame)
				{
					s_nextFrame.Add(coroutine);
				}
			}
			else
				throw new ArgumentNullException(nameof(coroutine));
		}

		/// <summary>
		/// Schedule an action to be run at the first frame at or after the specified time
		/// </summary>
		/// <param name="coroutine">Action to execute</param>
		/// <param name="time">Time when it should be executed, see <see cref="CurrentTime"/></param>
		public static void Schedule(Action coroutine, ulong time)
		{
			if (coroutine != null)
			{
				lock (s_queue)
				{
					// linear ordered insert, performance improvement might be a binary tree (i.e.: priority queue)
					for (var it = s_queue.First; it != null; it = it.Next)
					{
						if (time < it.Value.Item1)
						{
							s_queue.AddBefore(it, new Tuple<ulong, Action>(time, coroutine));
							return;
						}
					}

					s_queue.AddLast(new Tuple<ulong, Action>(time, coroutine));
				}
			}
			else
				throw new ArgumentNullException(nameof(coroutine));
		}

		/// <summary>
		/// Execute all scheduled coroutines
		/// </summary>
		internal static void Update()
		{
			ulong timeNow = CurrentTime;

			// all next frame coroutines
			{
				s_nextFrameProcessing = Interlocked.Exchange(ref s_nextFrame, s_nextFrameProcessing);

				for (int i = 0; i < s_nextFrameProcessing.Count; i++)
				{
					try
					{
						s_nextFrameProcessing[i]();
					}
					catch (Exception ex)
					{
						Debug.WriteLine(ex.ToString());
					}
				}

				s_nextFrameProcessing.Clear();
			}

			lock (s_queue)
			{
				// scheduled coroutines (ordered)
				for (var it = s_queue.First; it != null;)
				{
					var curIt = it;
					it = curIt.Next;

					if (curIt.Value.Item1 <= timeNow)
					{
						try
						{
							curIt.Value.Item2();
						}
						catch (Exception ex)
						{
							Debug.WriteLine(ex.ToString());
						}
						finally
						{
							s_queue.Remove(curIt);
						}
					}
					else
						return;
				}
			}
		}

		/// <summary>
		/// Time in milliseconds when the next action needs to be activated
		/// </summary>
		/// <returns>Delay in milliseconds or ~0ul (<see cref="ulong.MaxValue"/>) if there's nothing to run</returns>
		internal static ulong NextTaskTime()
		{
			if (s_nextFrame.Count != 0)
			{
				return CurrentTime;
			}
			else if (s_queue.Count != 0)
			{
				return s_queue.First.Value.Item1;
			}
			
			return ulong.MaxValue;
		}
	}
}
