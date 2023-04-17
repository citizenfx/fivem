using System;
using System.Threading;
using System.Collections.Generic;

namespace CitizenFX.Core
{
	public class Scheduler
	{
		internal static uint TimeNow { get; private set; }
		private static LinkedList<Tuple<uint, Action>> s_queue = new LinkedList<Tuple<uint, Action>>();

		// optimized non ordering
		private static List<Action> s_nextFrame = new List<Action>();
		private static List<Action> s_nextFrameProcessing = new List<Action>();

		public static void Schedule(Action coroutine)
		{
			s_nextFrame.Add(coroutine);
		}

		public static void Schedule(Action delay, uint time)
		{
			// linear ordered insert, performance improvement might be a binary tree (i.e.: priority queue)
			for (var it = s_queue.First; it != null; it = it.Next)
			{
				if (time < it.Value.Item1)
				{
					s_queue.AddBefore(it, new Tuple<uint, Action>(time, delay));
					return;
				}
			}

			s_queue.AddLast(new Tuple<uint, Action>(time, delay));
		}

		internal static void Update()
		{
			uint timeNow = TimeNow = (uint)Environment.TickCount;

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
}
