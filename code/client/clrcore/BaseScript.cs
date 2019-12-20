using System;
using System.Collections.Generic;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security;
using System.Text;
using System.Threading.Tasks;

using CitizenFX.Core.Native;

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

		protected internal EventHandlerDictionary EventHandlers { get; private set; }

		protected ExportDictionary Exports { get; private set; }

		[ThreadStatic]
		private static string ms_curName = null;

		internal static string CurrentName
		{
			get => ms_curName;
			set => ms_curName = value;
		}

#if !IS_FXSERVER && !IS_RDR3
		private Player m_player;

		protected Player LocalPlayer
		{
			get
			{
				var id = API.PlayerId();

				if (m_player == null || id != m_player.Handle)
				{
					m_player = new Player(id);
				}

				return m_player;
			}
		}
#endif

#if !IS_RDR3
		protected PlayerList Players { get; private set; }
#endif

		protected BaseScript()
		{
			EventHandlers = new EventHandlerDictionary();
			Exports = new ExportDictionary();
			CurrentTaskList = new Dictionary<Delegate, Task>();
#if !IS_RDR3
			Players = new PlayerList();
#endif
		}

		internal void ScheduleRun()
		{
			if (Tick != null)
			{
				var calls = Tick.GetInvocationList();

				foreach (var call in calls)
				{
					ScheduleTick(call);
				}
			}
		}

		internal void RegisterTick(Func<Task> tick)
		{
			Tick += tick;
		}

		internal void RegisterEventHandler(string eventName, Delegate callback)
		{
			EventHandlers[eventName] += callback;
		}

		internal void ScheduleTick(Delegate call)
		{
			if (!CurrentTaskList.ContainsKey(call))
			{
				var curName = $"{GetType().Name} -> tick {call.GetMethodInfo().Name}";

				try
				{
					ms_curName = curName;

					using (var scope = new ProfilerScope(() => curName))
					{
						CurrentTaskList.Add(call, CitizenTaskScheduler.Factory.StartNew(() =>
						{
							ms_curName = curName;

							try
							{
								using (var innerScope = new ProfilerScope(() => curName))
								{
									var t = ((Func<Task>)call)();

									return t;
								}
							}
							finally
							{
								ms_curName = null;
							}
						}).Unwrap().ContinueWith(a =>
						{
							if (a.IsFaulted)
							{
								Debug.WriteLine($"Failed to run a tick for {GetType().Name}: {a.Exception?.InnerExceptions.Aggregate("", (b, s) => s + b.ToString() + "\n")}");
							}

							CurrentTaskList.Remove(call);
						}));
					}
				}
				finally
				{
					ms_curName = null;
				}
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
		public static Task Delay(int msecs)
		{
			return CitizenTaskScheduler.Factory.FromAsync(BeginDelay, EndDelay, msecs, CitizenTaskScheduler.Instance);
		}

		[SecuritySafeCritical]
		public static void TriggerEvent(string eventName, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			TriggerEventInternal(eventName, argsSerialized, false);
		}

#if !IS_FXSERVER
		[SecuritySafeCritical]
		public static void TriggerServerEvent(string eventName, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			TriggerEventInternal(eventName, argsSerialized, true);
		}

		[SecuritySafeCritical]
		public static void TriggerLatentServerEvent(string eventName, int bytesPerSecond, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			TriggerLatentServerEventInternal(eventName, argsSerialized, bytesPerSecond);
		}
#else
		public static void TriggerClientEvent(Player player, string eventName, params object[] args)
		{
			player.TriggerEvent(eventName, args);
		}

		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerClientEvent(string eventName, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					Function.Call(Hash.TRIGGER_CLIENT_EVENT_INTERNAL, eventName, "-1", serialized, argsSerialized.Length);
				}
			}
		}

		public static void TriggerLatentClientEvent(Player player, string eventName, int bytesPerSecond, params object[] args)
		{
			player.TriggerLatentEvent(eventName, bytesPerSecond, args);
		}

		/// <summary>
		/// Broadcasts an event to all connected players.
		/// </summary>
		/// <param name="eventName">The name of the event.</param>
		/// <param name="args">Arguments to pass to the event.</param>
		public static void TriggerLatentClientEvent(string eventName, int bytesPerSecond, params object[] args)
		{
			var argsSerialized = MsgPackSerializer.Serialize(args);

			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					Function.Call(Hash.TRIGGER_LATENT_CLIENT_EVENT_INTERNAL, eventName, "-1", serialized, argsSerialized.Length, bytesPerSecond);
				}
			}
		}
#endif

#if !IS_FXSERVER
		[SecurityCritical]
		private static void TriggerLatentServerEventInternal(string eventName, byte[] argsSerialized, int bytesPerSecond)
		{
			var nativeHash = Hash.TRIGGER_LATENT_SERVER_EVENT_INTERNAL;
				
			unsafe
			{
				fixed (byte* serialized = &argsSerialized[0])
				{
					Function.Call(nativeHash, eventName, serialized, argsSerialized.Length, bytesPerSecond);
				}
			}
		}
#endif

		[SecurityCritical]
		private static void TriggerEventInternal(string eventName, byte[] argsSerialized, bool isRemote)
		{
			try
			{
				if (GameInterface.SnapshotStackBoundary(out var b))
				{
					InternalManager.ScriptHost.SubmitBoundaryEnd(b, b.Length);
				}

				var nativeHash = Hash.TRIGGER_EVENT_INTERNAL;

#if !IS_FXSERVER
				if (isRemote)
				{
					nativeHash = Hash.TRIGGER_SERVER_EVENT_INTERNAL;
				}
#endif

				unsafe
				{
					fixed (byte* serialized = &argsSerialized[0])
					{
						Function.Call(nativeHash, eventName, serialized, argsSerialized.Length);
					}
				}

				PreventTailCall();
			}
			finally
			{
				InternalManager.ScriptHost.SubmitBoundaryEnd(null, 0);
			}
		}

		[MethodImpl(MethodImplOptions.NoInlining)]
		private static void PreventTailCall()
		{

		}

		private static IAsyncResult BeginDelay(int delay, AsyncCallback callback, object state)
		{
			InternalManager.AddDelay(delay, callback, ms_curName);

			return new DummyAsyncResult();
		}

		private static void EndDelay(IAsyncResult result)
		{

		}

		public static void RegisterScript(BaseScript script)
		{
			InternalManager.AddScript(script);
		}

		public static void UnregisterScript(BaseScript script)
		{
			InternalManager.RemoveScript(script);
		}

		private bool m_initialized = false;

		internal void InitializeOnAdd()
		{
			if (m_initialized)
			{
				return;
			}

			m_initialized = true;

			var allMethods = this.GetType().GetMethods(BindingFlags.Public | BindingFlags.NonPublic | BindingFlags.Static | BindingFlags.Instance);

			IEnumerable<MethodInfo> GetMethods(Type t)
			{
				return allMethods.Where(m => m.GetCustomAttributes(t, false).Length > 0);
			}

			// register all Tick decorators
			try
			{
				foreach (var method in GetMethods(typeof(TickAttribute)))
				{
#if !IS_FXSERVER
					Debug.WriteLine("Registering Tick for attributed method {0}", method.Name);
#endif

					if (method.IsStatic)
						this.RegisterTick((Func<Task>)Delegate.CreateDelegate(typeof(Func<Task>), method));
					else
						this.RegisterTick((Func<Task>)Delegate.CreateDelegate(typeof(Func<Task>), this, method.Name));
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine("Registering Tick failed: {0}", e.ToString());
			}

			// register all EventHandler decorators
			try
			{
				foreach (var method in GetMethods(typeof(EventHandlerAttribute)))
				{
					var parameters = method.GetParameters().Select(p => p.ParameterType).ToArray();
					var actionType = Expression.GetDelegateType(parameters.Concat(new[] { typeof(void) }).ToArray());
					var attribute = method.GetCustomAttribute<EventHandlerAttribute>();

#if !IS_FXSERVER
					Debug.WriteLine("Registering EventHandler {2} for attributed method {0}, with parameters {1}", method.Name, string.Join(", ", parameters.Select(p => p.GetType().ToString())), attribute.Name);
#endif

					if (method.IsStatic)
						this.RegisterEventHandler(attribute.Name, Delegate.CreateDelegate(actionType, method));
					else
						this.RegisterEventHandler(attribute.Name, Delegate.CreateDelegate(actionType, this, method.Name));
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine("Registering EventHandler failed: {0}", e.ToString());
			}

			// register all commands
			try
			{
				foreach (var method in GetMethods(typeof(CommandAttribute)))
				{
					var attribute = method.GetCustomAttribute<CommandAttribute>();
					var parameters = method.GetParameters();

#if !IS_FXSERVER
					Debug.WriteLine("Registering command {0}", attribute.Command);
#endif

					// no params, trigger only
					if (parameters.Length == 0)
					{
						if (method.IsStatic)
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(null, null);
							}), attribute.Restricted);
						}
						else
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(this, null);
							}), attribute.Restricted);
						}
					}
					// Player
#if !IS_RDR3
					else if (parameters.Any(p => p.ParameterType == typeof(Player)) && parameters.Length == 1)
					{
#if IS_FXSERVER
						if (method.IsStatic)
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(null, new object[] { new Player(source.ToString()) });
							}), attribute.Restricted);
						}
						else
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(this, new object[] { new Player(source.ToString()) });
							}), attribute.Restricted);
						}
#else
						Debug.WriteLine("Client commands with parameter type Player not supported");
#endif
					}
#endif
					// string[]
					else if (parameters.Length == 1)
					{
						if (method.IsStatic)
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(null, new object[] { args.Select(a => (string)a).ToArray() });
							}), attribute.Restricted);
						}
						else
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(this, new object[] { args.Select(a => (string)a).ToArray() });
							}), attribute.Restricted);
						}
					}
					// Player, string[]
#if !IS_RDR3
					else if (parameters.Any(p => p.ParameterType == typeof(Player)) && parameters.Length == 2)
					{
#if IS_FXSERVER
						if (method.IsStatic)
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(null, new object[] { new Player(source.ToString()), args.Select(a => (string)a).ToArray() });
							}), attribute.Restricted);
						}
						else
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(this, new object[] { new Player(source.ToString()), args.Select(a => (string)a).ToArray() });
							}), attribute.Restricted);
						}
#else
						Debug.WriteLine("Client commands with parameter type Player not supported");
#endif
					}
#endif
					// legacy --> int, List<object>, string
					else
					{
						if (method.IsStatic)
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(null, new object[] { source, args, rawCommand });
							}), attribute.Restricted);
						}
						else
						{
							Native.API.RegisterCommand(attribute.Command, new Action<int, List<object>, string>((source, args, rawCommand) =>
							{
								method.Invoke(this, new object[] { source, args, rawCommand });
							}), attribute.Restricted);
						}
					}
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine("Registering command failed: {0}", e.ToString());
			}
		}
	}

	class DummyAsyncResult : IAsyncResult
	{
		public object AsyncState => null;

		public System.Threading.WaitHandle AsyncWaitHandle => null;

		public bool CompletedSynchronously => false;

		public bool IsCompleted => false;
	}

#if test
	class TestScript : BaseScript
    {
        public TestScript()
        {
            EventHandlers["getMapDirectives"] += new Action<dynamic>(add => 
            {
                Func<dynamic, string, Action<string>> addCB = (state, key) =>
                {
                    Debug.WriteLine("adding key {0}", key);

                    return new Action<string>(value =>
                    {
                        Debug.WriteLine("and key + value = {0} {1}", key, value);

                        state.add("key", key);
                    });
                };

                Action<dynamic> removeCB = (state) =>
                {
                    Debug.WriteLine("removing key {0}", state.key);
                };

                add("banana", addCB, removeCB);
            });

            Tick += TestScript_Tick;

            EventHandlers["chatMessage"] += new Action<dynamic, dynamic, dynamic>((name, color, text) =>
            {
                Debug.WriteLine("hmm, nice bike!45? " + text.ToString());

                CitizenFX.Core.UI.Screen.ShowNotification($"oi m9 - {text}");

                if (text.Contains("freeze"))
                {
                    Debug.WriteLine("hmm, nice bike!4534!!!");
                    Game.PlayerPed.IsPositionFrozen = !Game.PlayerPed.IsPositionFrozen;
                    /*var vehicle = await World.CreateVehicle("infernus", Game.PlayerPed.Position, 120.0f);
                    vehicle.Mods.LicensePlate = await Game.GetUserInput(8);
                    vehicle.Doors[VehicleDoorIndex.FrontLeftDoor].Open(instantly: true);*/

                    Debug.WriteLine("hmm, nice bike!!356457!!");

                    TriggerEvent("chatMessage", "heya", new int[] { 255, 0, 0 }, "i'm frozen!!");
                }
                else if (text.Contains("spown"))
                {
                    Exports["spawnmanager"].spawnPlayer(new
                    {
                        model = new Model("player_one").Hash,
                        x = -31.010f,
                        y = 6316.830f,
                        z = 40.083f,
                        heading = 180.0f
                    }, new Action<dynamic>(s =>
                    {
                        Debug.WriteLine("omg cb?");
                        Debug.WriteLine($"spawned on {s.x}");

                        TriggerEvent("chatMessage", "heya", new int[] { 255, 0, 0 }, $"wew {s.model}");
                    }));
                }
            });
        }

        async Task TestScript_Tick()
        {
            await Delay(1000);
            CitizenFX.Core.UI.Screen.ShowNotification($"oi m8!!! {Game.Player.Name} @ {DateTime.Now.ToLongTimeString()}");
        }
    }
#endif
}
