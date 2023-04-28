using System;
using System.Collections.Generic;
using System.Linq;

using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	internal static class EventsManager
	{
		private static Dictionary<string, List<Tuple<DynFunc, Binding>>> s_eventHandlers = new Dictionary<string, List<Tuple<DynFunc, Binding>>>();

#if IS_FXSERVER
		// We need this to ensure that server-internal events can't be triggered from the client as unlike 
		// Lua and JS, C# has never had net event filtering so adding it now would break compatibility.
		private static HashSet<string> s_serverInternalNetEvents = new HashSet<string>
		{
			"playerConnecting",
			"playerDropped",
			"playerJoining",
		};
#endif

		internal static unsafe void IncomingEvent(string eventName, string sourceString, Binding origin, byte* argsSerialized, int serializedSize, object[] args)
		{
#if IS_FXSERVER
			if (!s_serverInternalNetEvents.Contains(eventName) || sourceString.StartsWith("internal-net"))
#endif
			{
				if (s_eventHandlers.TryGetValue(eventName, out var delegateList))
				{
					// #TODO: should we silently ignore this or annoy peeps with it?
					if (args is null)
						args = MsgPackDeserializer.DeserializeArray(argsSerialized, serializedSize, sourceString);

					if (args != null)
					{
						Remote remote = new Remote(origin, sourceString);

						// shedule it for next update
						Scheduler.Schedule(() =>
						{
							for (int i = 0; i < delegateList.Count; ++i)
							{
								try
								{
									var ev = delegateList[i];
									if ((ev.Item2 & origin) != 0)
									{
										var result = ev.Item1(remote, args);
										if (result != null)
											return;
									}
								}
								catch (Exception ex)
								{
									if (!(ex is InvalidCastException) || Debug.LogInvalidCastExceptionsOnDynFunc)
									{
										string argsString = string.Join<string>(", ", args.Select(a => a != null ? a.GetType().ToString() : "null"));
										Debug.WriteLine($"^1Error while handling event: {eventName}\n\twith arguments: ({argsString})^7");
										Debug.PrintError(ex);
									}
								}
							}
						});
					}
				}
			}
		}

		#region Registration
		internal static void AddEventHandler(string eventName, DynFunc del, Binding binding = Binding.Local)
		{
			if (!s_eventHandlers.TryGetValue(eventName, out var delegateList))
			{
				delegateList = new List<Tuple<DynFunc, Binding>>();
				s_eventHandlers.Add(eventName, delegateList);

				CoreNatives.RegisterResourceAsEventHandler(eventName);
			}

			delegateList.Add(new Tuple<DynFunc, Binding>(del, binding));
		}

		internal static void RemoveEventHandler(string eventName, Delegate del)
		{
			if (s_eventHandlers.TryGetValue(eventName, out var delegateList))
			{
				int index = delegateList.FindIndex(cur => cur.Item1.Equals(del));
				if (index != -1)
				{
					s_eventHandlers[eventName].RemoveAt(index);
				}
			}
		}
		#endregion
	}

	public class EventHandler : Dictionary<string, EventHandlerSet>
	{
		public new EventHandlerSet this[string key]
		{
			get
			{
				var lookupKey = key.ToLower();

				if (!TryGetValue(lookupKey, out var entry))
				{
					entry = new EventHandlerSet(key);
					base.Add(lookupKey, entry);
				}

				return entry;
			}
			set { /* ignore, this will enable += syntax */ }
		}

		public void Add(string key, DynFunc value) => this[key].Add(value);
	}

	public class EventHandlerSet
	{
		private readonly string m_eventName;
		private readonly List<DynFunc> m_handlers = new List<DynFunc>();

		public EventHandlerSet(string eventName)
		{
			m_eventName = eventName;
		}

		~EventHandlerSet()
		{
			for(int i = 0; i < m_handlers.Count; ++i)
			{
				EventsManager.RemoveEventHandler(m_eventName, m_handlers[i]);
			}
		}

		public EventHandlerSet Add(DynFunc deleg, Binding binding = Binding.Local)
		{
			m_handlers.Add(deleg);
			EventsManager.AddEventHandler(m_eventName, deleg, binding);
			return this;
		}

		public EventHandlerSet Remove(Delegate deleg)
		{
			int index = m_handlers.FindIndex(cur => deleg.Equals(cur));
			if (index != -1)
			{
				m_handlers.RemoveAt(index);
				EventsManager.RemoveEventHandler(m_eventName, deleg);
			}
			return this;
		}

		public static EventHandlerSet operator +(EventHandlerSet entry, DynFunc deleg) => entry.Add(deleg);

		public static EventHandlerSet operator -(EventHandlerSet entry, DynFunc deleg) => entry.Remove(deleg);

		/// <summary>
		/// Backwards compatibility
		/// </summary>
		/// <param name="entry"></param>
		/// <param name="deleg"></param>
		/// <returns></returns>
		[Obsolete("This is slow, use += Func.Create<T..., Ret>(method) instead.", false)]
		public static EventHandlerSet operator +(EventHandlerSet entry, Delegate deleg) => entry.Add((remote, args) => deleg.DynamicInvoke(args));

		/// <summary>
		/// Backwards compatibility
		/// </summary>
		/// <param name="entry"></param>
		/// <param name="deleg"></param>
		/// <returns></returns>
		public static EventHandlerSet operator -(EventHandlerSet entry, Delegate deleg) => entry.Remove(deleg);
	}
}
