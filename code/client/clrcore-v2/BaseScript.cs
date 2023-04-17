using CitizenFX.Core.Native;
using System;
using System.Collections.Generic;
using System.Reflection;
using System.Security;

namespace CitizenFX.Core
{
	public abstract class BaseScript
	{
		#region Fields

		private bool m_initialized = false;

		private readonly List<CoroutineRepeat> m_tickList = new List<CoroutineRepeat>();

#if REMOTE_FUNCTION_ENABLED
		private readonly List<RemoteHandler> m_persistentFunctions = new List<RemoteHandler>();
#endif
		protected EventHandler EventHandlers => new EventHandler();

		protected Exports Exports => new Exports();

		#endregion

		#region Instance Initialization
		~BaseScript()
		{
			for (int i = 0; i < m_tickList.Count; ++i)
				m_tickList[i].Stop();
		}

		[SecuritySafeCritical]
		internal void Initialize()
		{
			if (m_initialized)
				return;

			m_initialized = true;

			var scriptMethods = this.GetType().GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance);
			foreach (MethodInfo method in scriptMethods)
			{
				var attributes = method.GetCustomAttributes(false);
				for (int a = 0; a < attributes.Length; ++a)
				{
					var attribute = attributes[a];

					try
					{
						switch (attribute)
						{
							case TickAttribute tick:
								Tick += (Func<Coroutine>)method.CreateDelegate(typeof(Func<Coroutine>), this);
								break;

							case EventHandlerAttribute eventHandler:
								RegisterEventHandler(eventHandler.Event, Func.Create(this, method), eventHandler.Binding);
								break;

							case CommandAttribute command:
								Native.CoreNatives.RegisterCommand(command.Command, Func.Create(this, method), command.Restricted);
								break;

							case ExportAttribute export:
								Exports.Add(export.Export, Func.Create(this, method), export.Binding);
								break;
						}
					}
					catch (Exception e)
					{
						Debug.WriteLine($"Registering {attribute.ToString().Replace("Attribute", "")} {method.DeclaringType.FullName}.{method.Name} failed with exception: {e}");
					}
				}
			}
		}

		#endregion

		#region Update/Tick Scheduling

		/// <summary>
		/// An event containing callbacks to attempt to schedule on every game tick.
		/// A callback will only be rescheduled once the associated task completes.
		/// </summary>
		protected event Func<Coroutine> Tick
		{
			add
			{
				lock (m_tickList)
				{
					CoroutineRepeat newTick = new CoroutineRepeat(value);
					m_tickList.Add(newTick);
					newTick.Schedule();
				}
			}
			remove
			{
				lock (m_tickList)
				{
					int index = m_tickList.FindIndex(th => th.Equals(value));
					if (index == -1)
					{
						m_tickList[index].Stop();
						m_tickList.RemoveAt(index);
					}
				}
			}
		}

		internal void RegisterTick(Func<Coroutine> tick) => Tick += tick;

		private void StartCoroutine(Action action) => Scheduler.Schedule(action);

		public static Coroutine WaitUntil(uint msecs) => Coroutine.WaitUntil(msecs);

		public static Coroutine WaitUntilNextFrame() => Coroutine.Yield();

		public static Coroutine Yield() => Coroutine.Yield();

		/// <summary>
		/// Returns a task that will delay scheduling of the current interval function by the passed amount of time.
		/// </summary>
		/// <example>
		/// await Delay(500);
		/// </example>
		/// <param name="msecs">The amount of time by which to delay scheduling this interval function.</param>
		/// <returns>An awaitable task.</returns>
		public static Coroutine Delay(uint msecs = 0u) => Coroutine.Delay(msecs);

		public static Coroutine Wait(uint msecs = 0u) => Delay(msecs);

		public static Coroutine WaitForSeconds(uint msecs) => Delay(msecs * 1000u);

		public static Coroutine WaitForMinutes(uint msecs) => Delay(msecs * 1000u * 60u);

		#endregion

		#region Events Handlers
		internal void RegisterEventHandler(string eventName, DynFunc deleg, Binding binding = Binding.LOCAL) => EventHandlers[eventName].Add(deleg, binding);
		#endregion

		#region Script loading

		public static void RegisterScript(BaseScript script)
		{
			ScriptManager.AddScript(script);
		}

		public static void UnregisterScript(in BaseScript script)
		{
			ScriptManager.RemoveScript(script);
		}

		#endregion

#if REMOTE_FUNCTION_ENABLED
		public RemoteHandler CreateRemoteHandler(Delegate deleg)
		{
			var persistentFunction = _RemoteHandler.Create(deleg);
			m_persistentFunctions.Add(persistentFunction);
			return persistentFunction;
		}

		public void RemoveRemoteHandler(RemoteHandler deleg)
		{
			var index = m_persistentFunctions.FindIndex(func => func == deleg);
			if (index > 0)
			{
				m_persistentFunctions.RemoveAt(index);

				if (deleg.Target != null && deleg.Target is _RemoteHandler pf)
				{
					ExternalsManager.UnRegisterRemoteFunction(pf.m_id);
				}
			}
		}
#endif
	}

#if !IS_FXSERVER
	public abstract class ClientScript : BaseScript { }
#else
	public abstract class ServerScript : BaseScript { }
#endif
}
