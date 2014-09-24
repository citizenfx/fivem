using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;

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

        private Player m_player;

        protected Player LocalPlayer
        {
            get
            {
                var id = Function.Call<int>(Natives.GET_PLAYER_ID);

                if (m_player == null || id != m_player.ID)
                {
                    m_player = new Player(id);
                }

                return m_player;
            }
        }

        protected PlayerList Players { get; private set; }

        public BaseScript()
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

        internal void ScheduleTick(Delegate call)
        {
            if (!CurrentTaskList.ContainsKey(call))
            {
                CurrentTaskList.Add(call, CitizenTaskScheduler.Factory.StartNew((Func<Task>)call).Unwrap().ContinueWith(a => CurrentTaskList.Remove(call)));
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
        protected Task Delay(int msecs)
        {
            return CitizenTaskScheduler.Factory.FromAsync(BeginDelay, EndDelay, msecs, null);
        }

        protected void TriggerEvent(string eventName, params object[] args)
        {
            var argsSerialized = MsgPackSerializer.Serialize(args);

            TriggerEventInternal(eventName, argsSerialized, false);
        }

        protected void TriggerServerEvent(string eventName, params object[] args)
        {
            var argsSerialized = MsgPackSerializer.Serialize(args);

            TriggerEventInternal(eventName, argsSerialized, true);
        }

        [SecuritySafeCritical]
        private void TriggerEventInternal(string eventName, byte[] argsSerialized, bool isRemote)
        {
            GameInterface.TriggerEvent(eventName, argsSerialized, isRemote);
        }

        private IAsyncResult BeginDelay(int delay, AsyncCallback callback, object state)
        {
            RuntimeManager.AddDelay(delay, callback);

            return new DummyAsyncResult();
        }

        private void EndDelay(IAsyncResult result)
        {
            
        }
    }

    class DummyAsyncResult : IAsyncResult
    {
        public object AsyncState { get { return null; } }

        public System.Threading.WaitHandle AsyncWaitHandle { get { return null; } }

        public bool CompletedSynchronously { get { return false; } }

        public bool IsCompleted { get { return false; } }
    }

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
        }

        async Task TestScript_Tick()
        {
            await Delay(1500);

            try
            {
                var playerId = Function.Invoke<int>(Natives.GET_PLAYER_ID);

                Pointer playerPed = typeof(int);
                Function.Invoke(Natives.GET_PLAYER_CHAR, playerId, playerPed);

                Pointer pX = typeof(float);
                Pointer pY = typeof(float);
                Pointer pZ = typeof(float);

                Function.Invoke(Natives.GET_CHAR_COORDINATES, (int)playerPed, pX, pY, pZ);

                Debug.WriteLine("coords: {0} {1} {2}", pX.Value, pY.Value, pZ.Value);
            }
            catch (Exception ex)
            {
                Debug.WriteLine("{0}", ex.ToString());
            }
        }
    }
}
