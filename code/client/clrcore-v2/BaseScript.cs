using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Reflection;
using System.Security;

namespace CitizenFX.Core
{
	public abstract class BaseScript
	{
		[Flags]
		internal enum State : byte
		{
			Uninitialized = 0x0,
			Initialized = 0x1,
			Enabled = 0x2,
		}

		#region Fields & Properties

		private State m_state = State.Uninitialized;

		public bool IsEnabled => (m_state & State.Enabled) != 0;

		private readonly List<CoroutineRepeat> m_tickList = new List<CoroutineRepeat>();

		/// <summary>
		/// A method to be scheduled on every game tick, do note that they'll only be rescheduled for the next frame once the method returns.
		/// </summary>
		protected event Func<Coroutine> Tick
		{
			add => RegisterTick(value);
			remove => UnregisterTick(value);
		}

		private readonly List<KeyValuePair<int, DynFunc>> m_commands = new List<KeyValuePair<int, DynFunc>>();
		
		private readonly Dictionary<string, DynFunc> m_nuiCallbacks = new Dictionary<string, DynFunc>();

#if REMOTE_FUNCTION_ENABLED
		private readonly List<RemoteHandler> m_persistentFunctions = new List<RemoteHandler>();
#endif
		protected EventHandler EventHandlers { get; } = new EventHandler();

		protected Exports Exports { get; } = new Exports();

		#endregion

		#region Instance initialization, finalization, and enablement
		~BaseScript()
		{
			if (!AppDomain.CurrentDomain.IsFinalizingForUnload())
			{
				// remove all reserved command slots
				for (int i = 0; i < m_commands.Count; ++i)
				{
					ReferenceFunctionManager.Remove(m_commands[i].Key);
				}

				m_commands.Clear(); // makes sure Disable() call below won't unnecessarily try to disable commands

				Disable();
			}
		}

		[SecuritySafeCritical]
		internal void Initialize()
		{
			if (m_state != State.Uninitialized
#if IS_FXSERVER
				|| this is ClientScript // shared-lib support: disallow client scripts to be loaded in server environments
#else
				|| this is ServerScript // shared-lib support: disallow server scripts to be loaded in client environments
#endif
				)
				return;

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
								RegisterTick((Func<Coroutine>)method.CreateDelegate(typeof(Func<Coroutine>), this), tick.StopOnException);
								break;

							case EventHandlerAttribute eventHandler:
								RegisterEventHandler(eventHandler.Event, Func.Create(this, method), eventHandler.Binding);
								break;

							case CommandAttribute command:
								RegisterCommand(command.Command, Func.CreateCommand(this, method, command.RemapParameters), command.Restricted);
								break;
#if !IS_FXSERVER
							case KeyMapAttribute keyMap:
								RegisterKeyMap(keyMap.Command, keyMap.Description, keyMap.InputMapper, keyMap.InputParameter, Func.CreateCommand(this, method, keyMap.RemapParameters));
								break;
							case NuiCallbackAttribute nuiCallback:
								RegisterNuiCallback(nuiCallback.CallbackName, Func.Create(this, method));
								break;
#endif
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

			m_state = State.Initialized | State.Enabled;
		}

		/// <summary>
		/// Enables all ticks, commands, events, and exports
		/// </summary>
		public void Enable()
		{
			if (m_state == State.Uninitialized)
			{
				Initialize();
				OnEnable();
			}
			else if ((m_state & State.Enabled) == 0)
			{
				// ticks
				for (int i = 0; i < m_tickList.Count; ++i)
				{
					m_tickList[i].Schedule();
				}

				// commands
				for (int i = 0; i < m_commands.Count; ++i)
				{
					ReferenceFunctionManager.SetDelegate(m_commands[i].Key, m_commands[i].Value);
				}

				foreach (var nuiCallback in m_nuiCallbacks)
				{
					Native.CoreNatives.RegisterNuiCallback(nuiCallback.Key, nuiCallback.Value);
				}
				
				EventHandlers.Enable();
				Exports.Enable();

				m_state |= State.Enabled;

				OnEnable();
			}
		}

		/// <summary>
		/// Disables all tick repeats, commands, events, and exports
		/// </summary>
		/// <remarks>
		/// 1. This <see cref="BaseScript"/> can't re-enable itself except for callbacks, you may want to hold a reference to it.<br />
		/// 2. External code/scripts can still call in for commands, but they'll get <see langword="null"/> returned automatically.
		/// </remarks>
		public void Disable()
		{
			if ((m_state & State.Enabled) != 0)
			{
				// ticks
				for (int i = 0; i < m_tickList.Count; ++i)
				{
					m_tickList[i].Stop();
				}

				// commands
				for (int i = 0; i < m_commands.Count; ++i)
				{
					ReferenceFunctionManager.SetDelegate(m_commands[i].Key, (_0, _1) => null);
				}
				
				foreach (var nuiCallback in m_nuiCallbacks)
				{
					Native.CoreNatives.RemoveNuiCallback(nuiCallback.Key);
				}

				EventHandlers.Disable();
				Exports.Disable();

				m_state &= ~State.Enabled;

				OnDisable();
			}
		}

		/// <summary>
		/// Disables all tick repeats, commands, events, and exports
		/// </summary>
		/// <remarks>
		/// 1. This <see cref="BaseScript"/> can't re-enable itself, unless <paramref name="deleteAllCallbacks"/> is <see langword="false" /> then callbacks are still able to, you may want to hold a reference to it.<br />
		/// 2. External code/scripts can still call in for commands, but they'll get <see langword="null"/> returned automatically.
		/// </remarks>
		/// <param name="deleteAllCallbacks">If enabled will delete all callbacks targeting this <see cref="BaseScript"/> instance</param>
		public void Disable(bool deleteAllCallbacks)
		{
			Disable(); // Remove commands' Target as well, so below `RemoveAllWithTarget` call won't delete those

			if (deleteAllCallbacks)
			{
				ReferenceFunctionManager.RemoveAllWithTarget(this);
			}
		}

		/// <summary>
		/// Called when this script got enabled
		/// </summary>
		protected virtual void OnEnable() { }

		/// <summary>
		/// Called when this script got disabled
		/// </summary>
		protected virtual void OnDisable() { }

		#endregion

		#region Update/Tick Scheduling

		public void RegisterTick(Func<Coroutine> tick, bool stopOnException = false)
		{
			lock (m_tickList)
			{
				CoroutineRepeat newTick = new CoroutineRepeat(tick, stopOnException);
				m_tickList.Add(newTick);
				newTick.Schedule();
			}
		}

		public void UnregisterTick(Func<Coroutine> tick)
		{
			lock (m_tickList)
			{
				for (int i = 0; i < m_tickList.Count; ++i)
				{
					var stored = m_tickList[i];
					if (stored.m_coroutine.Method == tick.Method && stored.m_coroutine.Target == tick.Target)
					{
						stored.Stop();
						m_tickList.RemoveAt(i);
						break;
					}
				}
			}
		}

		public static Coroutine WaitUntil(TimePoint msecs) => Coroutine.WaitUntil(msecs);

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

		public static Coroutine WaitForSeconds(uint seconds) => Delay(seconds * 1000u);

		public static Coroutine WaitForMinutes(uint minutes) => Delay(minutes * 1000u * 60u);

		#endregion

		#region Events & Command registration
		internal void RegisterEventHandler(string eventName, DynFunc deleg, Binding binding = Binding.Local) => EventHandlers[eventName].Add(deleg, binding);
		internal void UnregisterEventHandler(string eventName, DynFunc deleg) => EventHandlers[eventName].Remove(deleg);

		internal void RegisterCommand(string command, DynFunc dynFunc, bool isRestricted = true)
			=> m_commands.Add(new KeyValuePair<int, DynFunc>(ReferenceFunctionManager.CreateCommand(command, dynFunc, isRestricted), dynFunc));

		internal void RegisterKeyMap(string command, string description, string inputMapper, string inputParameter, DynFunc dynFunc)
		{
#if !GTA_FIVE
			throw new NotImplementedException();
#else
			if (inputMapper != null && inputParameter != null)
			{
				Native.CoreNatives.RegisterKeyMapping(command, description, inputMapper, inputParameter);
			}
			m_commands.Add(new KeyValuePair<int, DynFunc>(ReferenceFunctionManager.CreateCommand(command, dynFunc, false), dynFunc));
#endif
		}
		
		#endregion
		
		#region NUI Callback registration

		internal void RegisterNuiCallback(string callbackName, DynFunc dynFunc)
		{
#if IS_FXSERVER
			throw new NotImplementedException();
#endif
			m_nuiCallbacks.Add(callbackName, dynFunc);
			Native.CoreNatives.RegisterNuiCallback(callbackName, dynFunc);
		}

		/// <summary>
		/// Registers the NUI callback and binds it to the the <see cref="BaseScript"/>
		/// </summary>
		/// <param name="callbackName">the NUI callback to add</param>
		/// <param name="delegateFn">The function to bind the callback to, the callback will be invoked with an ExpandoObject containing the data and a <see cref="Callback"/> </param>
#if IS_FXSERVER
		/// <summary>Does nothing on server side</summary>
		[EditorBrowsable(EditorBrowsableState.Never)]
#endif
		public void RegisterNuiCallback(string callbackName, Delegate delegateFn)
		{
#if IS_FXSERVER
			throw new NotImplementedException();
#endif
			DynFunc dynFunc = Func.Create(delegateFn);
			m_nuiCallbacks.Add(callbackName, dynFunc);
			Native.CoreNatives.RegisterNuiCallback(callbackName, dynFunc);
		}
		
		/// <summary>
		/// Unregisters the NUI callback from the <see cref="BaseScript"/>
		/// </summary>
		/// <param name="callbackName">the NUI callback to remove</param>
#if IS_FXSERVER
		/// <summary>Does nothing on server side</summary>
		[EditorBrowsable(EditorBrowsableState.Never)]
#endif
		public void UnregisterNuiCallback(string callbackName)
		{
#if IS_FXSERVER
			throw new NotImplementedException();
#endif
			m_nuiCallbacks.Remove(callbackName);
			Native.CoreNatives.RemoveNuiCallback(callbackName);
		}
		
		/// <summary>
		/// Registers a NUI callback to the specified <paramref name="callbackName"/>
		/// </summary>
		/// <param name="callbackName">the event that the callback will bind to</param>
		/// <param name="delegateFn">The function to bind the callback to, the callback will be invoked with an ExpandoObject containing the data and a <see cref="Callback"/> </param>
#if IS_FXSERVER
		/// <summary>Does nothing on server side</summary>
		[EditorBrowsable(EditorBrowsableState.Never)]
#endif
		public static void AddNuiCallback(string callbackName, Delegate delegateFn)
		{
#if IS_FXSERVER
			throw new NotImplementedException();
#endif
			Native.CoreNatives.RegisterNuiCallback(callbackName, delegateFn);
		}

		/// <summary>
		/// Removes the NUI callback for the specified <paramref namem="callbackName"/>
		/// </summary>
		/// <param name="callbackName">the callback event that will be removed</param>
#if IS_FXSERVER
		/// <summary>Does nothing on server side</summary>
		[EditorBrowsable(EditorBrowsableState.Never)]
#endif
		public static void RemoveNuiCallback(string callbackName) 
		{
#if IS_FXSERVER
			throw new NotImplementedException();
#else
			Native.CoreNatives.RemoveNuiCallback(callbackName);
#endif
		}
		
		#endregion

		#region Script loading

		/// <summary>
		/// Activates all ticks, events, and exports.
		/// </summary>
		/// <param name="script">script to activate</param>
		public static void RegisterScript(BaseScript script)
		{
			ScriptManager.AddScript(script);
		}

		/// <summary>
		/// Deactivates all ticks, events, and exports.
		/// </summary>
		/// <param name="script">script to deactivate</param>
		public static void UnregisterScript(BaseScript script)
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

	/// <inheritdoc cref="BaseScript"/>
	/// <remarks>Will and can only be activated in client environments</remarks>
#if IS_FXSERVER
	[EditorBrowsable(EditorBrowsableState.Never)]
#endif
	public abstract class ClientScript : BaseScript { }

	/// <inheritdoc cref="BaseScript"/>
	/// <remarks>Will and can only be activated in server environments</remarks>
#if !IS_FXSERVER
	[EditorBrowsable(EditorBrowsableState.Never)]
#endif
	public abstract class ServerScript : BaseScript { }
}
