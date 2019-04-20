using System;
using System.Collections.Generic;
using System.Linq;
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

#if !IS_FXSERVER
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

		protected PlayerList Players { get; private set; }

		protected BaseScript()
		{
			EventHandlers = new EventHandlerDictionary();
			Exports = new ExportDictionary();
			CurrentTaskList = new Dictionary<Delegate, Task>();

			Players = new PlayerList();
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
				CurrentTaskList.Add(call, Task.Factory.StartNew((Func<Task>)call).Unwrap().ContinueWith(a =>
				{
					if (a.IsFaulted)
					{
						Debug.WriteLine($"Failed to run a tick for {GetType().Name}: {a.Exception?.InnerExceptions.Aggregate("", (b, s) => s + b.ToString() + "\n")}");
					}

					CurrentTaskList.Remove(call);
				}));
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
			return Task.Factory.FromAsync(BeginDelay, EndDelay, msecs, null);
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
#endif

		[SecurityCritical]
		private static void TriggerEventInternal(string eventName, byte[] argsSerialized, bool isRemote)
		{
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
		}

		private static IAsyncResult BeginDelay(int delay, AsyncCallback callback, object state)
		{
			InternalManager.AddDelay(delay, callback);

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
