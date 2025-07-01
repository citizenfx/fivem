using CitizenFX.Core.Native;
using CitizenFX.MsgPack;
using CitizenFX.Shared;
using System;
using System.Collections.Concurrent;
using System.Threading;

namespace CitizenFX.Core
{
	/* Callbacks internally handled for FiveM
	 * 
	 * In case we want to extend this to other ScRTs in the future, 
	 * here's a very simple explanation of how it works.
	 *
	 * The functionality is straightforward: a callback is registered like a normal event, 
	 * with the only difference being that it returns a value.
	 *
	 * When triggered, two events are awaited:
	 * - __cfx_cbrequest: sent by the calling side to request the callback execution on the remote side.
	 * - __cfx_cbresponse: sent by the responding side once the result is ready, returning the value to the caller.
	 *
	 * A helper class handles the request, providing all the necessary information to the remote side.
	 * `CallbackInfoDelegate` is a simple structure that carries the event name, request ID, and optional data
	 * to the remote callback.
	 *
	 * When returning data, only the request ID (to ensure we reply to the correct caller) and the return value are sent.
	 *
	 * What data is exchanged?
	 * 
	 * - Triggering the callback:
	 *   - `RequestID`: a unique identifier for each request, ensuring that the response goes to the correct caller 
	 *                  (even with concurrent requests to the same callback).
	 *   - `EventName`: a readable name for the callback, similar to how we name events and exports.
	 *   - `Args`: optional data to be passed to the remote callback for processing before returning the value.
	 * 
	 * - Replying to the callback:
	 *   - `RequestID`: used to match the response to the original request, preventing concurrency issues.
	 *   - `ReturnValue`: the actual data we want to receive.
	 *
	 * How is the callback system managed?
	 * 
	 * We use two `ConcurrentDictionary` instances:
	 * - `_handlers`: contains the registered callbacks, similar to how event handlers are stored.
	 * - `_pending`: holds the pending responses. When a callback is triggered, an entry is added to `_pending`.
	 *               Once a response is received (or a timeout occurs), the entry is removed.
	 *               Ideally, `_pending` should remain as empty as possible and be cleared as quickly as possible.
	 */


	internal static class CallbacksManager
	{
		private static readonly ConcurrentDictionary<string, MsgPackFunc> _handlers = new ConcurrentDictionary<string, MsgPackFunc>();
		private static readonly ConcurrentDictionary<string, MsgPackFunc> _pending = new ConcurrentDictionary<string, MsgPackFunc>();
		private static int _requestCounter; // simple counter to handle uniqueIDs
		private const string CallbackRequestEvent = "__cfx_internal:cbrequest";
		private const string CallbackResponseEvent = "__cfx_internal:cbresponse";
		private const int TimeoutMilliseconds = 5000; // too much? callbacks should never take > 1000ms.. and imho 1s is waaay too much to wait for a response from otherside.
		internal delegate TResult TypeDeserializer<out TResult>(ref MsgPackDeserializer arg);

		static CallbacksManager()
		{
			CoreNatives.RegisterResourceAsEventHandler(CallbackResponseEvent);
			CoreNatives.RegisterResourceAsEventHandler(CallbackRequestEvent);
		}

		internal static unsafe bool IncomingRequest(string eventName, string sourceString, Binding origin, byte* argsSerialized, int serializedSize)
		{
			Remote remote = new Remote(origin, sourceString);
			MsgPackDeserializer deserializer = new MsgPackDeserializer(argsSerialized, (ulong)serializedSize, origin == Binding.Remote ? sourceString : null);
			var restorePoint = deserializer.CreateRestorePoint();

			if (eventName == CallbackResponseEvent)
			{
				uint length = deserializer.ReadArraySize();
				string requestId = ReadStringBytes(ref deserializer);
#if !IS_FXSERVER
				if(requestId.StartsWith("srv"))
					return true;
#else
				if (requestId.StartsWith("cli"))
					return true;
#endif
				// we create a new deserializer only for the array of arguments that
				// we receive from the callback to execute it.
				var obj = deserializer.DeserializeAsObject();
				byte[] args = MsgPackSerializer.SerializeToByteArray(obj);
				fixed (byte* argsPtr = args)
				{
					deserializer = new MsgPackDeserializer(argsPtr, (ulong)args.Length, origin == Binding.Remote ? sourceString : null);

					if (_pending.TryRemove(requestId, out var coro))
					{
						try
						{
							var result = coro(remote, ref deserializer);
							if (result is bool _b && !_b)
								return false;
						}
						catch (Exception ex)
						{
							var objs = deserializer.DeserializeAsObjectArray();
							Debug.WriteException(ex, coro, objs, "callbacks manager");
						}
					}
				}
				return true;
			}
			else if (eventName == CallbackRequestEvent)
			{
				// we always send an array.. even for only 1 obj
				uint length = deserializer.ReadArraySize();

				var deleF = MsgPackRegistry.GetOrCreateDeserializer(typeof(CallbackInfoDelegate)).CreateDelegate(typeof(TypeDeserializer<CallbackInfoDelegate>));
				var callbackInfo = ((TypeDeserializer<CallbackInfoDelegate>)deleF)(ref deserializer);

				string requestId = callbackInfo.requestId;
#if !IS_FXSERVER
				if(requestId.StartsWith("cli"))
					return true;
#else
				if (requestId.StartsWith("srv"))
					return true;
#endif
				string evName = callbackInfo.evName;
				string playerHandle = string.Empty;
#if IS_FXSERVER
				playerHandle = remote.m_playerId ?? string.Empty;
#endif

				if (_handlers.TryGetValue(evName, out var handler))
				{
					try
					{
						byte[] args = MsgPackSerializer.SerializeToByteArray(callbackInfo.args);
						fixed(byte* argsPtr = args)
						{
							// we create a new deserializer only for the array of arguments that
							// we receive from the InfoDelegate (MsgPackFunc wants an array of args)
							deserializer = new MsgPackDeserializer(argsPtr, (ulong)args.Length, origin == Binding.Remote ? sourceString : null);
							var result = handler(remote, ref deserializer);

							if (result is Coroutine coro)
								coro.ContinueWith(r => SendResponse(CallbackResponseEvent, playerHandle, origin,requestId,r.GetResultNonThrowing()));
							else
								((Coroutine)result).ContinueWith(r => SendResponse(CallbackResponseEvent, playerHandle, origin, requestId, r.GetResultNonThrowing()));
						}
					}
					catch (Exception ex)
					{

						Debug.WriteException(ex, handler, callbackInfo.args, "callbacks manager");
						//Send(CallbackResponseEvent,playerHandle,origin,requestId,null);
					}
				}
				return true;
			}

			return false;
		}

		internal static void Register(string name, Binding binding, MsgPackFunc handler)
		{
			if (!_handlers.TryAdd(name, (Remote remote, ref MsgPackDeserializer deserializer) =>
			{
				var coro = handler(remote, ref deserializer);
				return coro;
			}))
			{
				throw new Exception($"Error while trying to register event callback [{name}].");
			}
		}

		internal static void Unregister(string name)
		{
			if(!_handlers.TryRemove(name, out MsgPackFunc val))
			{
				throw new Exception($"Error while trying to unregister event callback [{name}].");
			}
		}

		internal static Coroutine<T> TriggerInternal<T>(Binding binding, string target, string name, params object[] args)
		{
			string requestId = GenerateRequestId(binding == Binding.Local);
			var coro = new Coroutine<T>();

			_pending[requestId] = (Remote r, ref MsgPackDeserializer d) =>
			{
				var deleg = MsgPackRegistry.GetOrCreateDeserializer(typeof(T)).CreateDelegate(typeof(TypeDeserializer<T>));
				object result = ((TypeDeserializer<T>)deleg)(ref d);
				coro.Complete((T)result);
				return null;
			};

			Send(CallbackRequestEvent, target, binding, requestId, name, args);

			// if we don't get a reply before the timeout, let's fail the coroutine and return.
			Scheduler.Schedule(() =>
			{
				if (_pending.TryRemove(requestId, out var pending))
				{
					coro.Fail(default(T), new TimeoutException($"Callback '{name}' timed out after {TimeoutMilliseconds}ms."));
					coro.GetResult();
				}
			}, TimeoutMilliseconds);

			return coro;
		}

		private static void Send(string eventName, string target, Binding binding, string requestId, string callbackName, object[] args)
		{

			CallbackInfoDelegate del = new CallbackInfoDelegate()
			{
				requestId = requestId,
				evName = callbackName,
				args = args
			};

			SendRequest(eventName, target, binding, del);
		}

		private static void SendResponse(string eventName, string target, Binding binding, string requestId, object result)
		{
			InPacket var = new InPacket(new[] { requestId, result });
			if (binding == Binding.Local)
			{
				CoreNatives.TriggerEventInternal(eventName, var);
			}
			else if (binding == Binding.Remote)
			{
#if IS_FXSERVER
				CoreNatives.TriggerClientEventInternal(eventName, target, var);
#else
				CoreNatives.TriggerServerEventInternal(eventName, var);
#endif
			}
		}

		private static void SendRequest(string eventName, string target, Binding binding, CallbackInfoDelegate del)
		{
			InPacket var = new InPacket(new[] { del });
			if (binding == Binding.Local)
			{
				CoreNatives.TriggerEventInternal(eventName, var);
			}
			else if (binding == Binding.Remote)
			{
#if IS_FXSERVER
				CoreNatives.TriggerClientEventInternal(eventName, target, var);
#else
				CoreNatives.TriggerServerEventInternal(eventName, var);
#endif
			}
			else
			{
				throw new NotSupportedException("Cannot trigger all clients for a callback."); // todo: make a better error message
			}
		}

		private static string GenerateRequestId(bool local)
		{
			int id = Interlocked.Increment(ref _requestCounter);
			if(local)
				return $"loc_{id}";
#if IS_FXSERVER
			return $"srv_{id}";
#else
			return $"cli_{id}";
#endif
		}

		private static string ReadStringBytes(ref MsgPackDeserializer deserializer)
		{
			MsgPackCode type = (MsgPackCode)deserializer.ReadByte();

			if (type <= MsgPackCode.FixStrMax)
			{
				if (type <= MsgPackCode.FixIntPositiveMax)
					return ((byte)type).ToString();
				else
					return deserializer.ReadString((byte)type % 32u);
			}
			else if (type >= MsgPackCode.FixIntNegativeMin) // anything at the end of our byte
			{
				return unchecked((sbyte)type).ToString();
			}
			switch (type)
			{
				case MsgPackCode.Str8: return deserializer.ReadString(deserializer.ReadUInt8());
				case MsgPackCode.Str16: return deserializer.ReadString(deserializer.ReadUInt16());
				case MsgPackCode.Str32: return deserializer.ReadString(deserializer.ReadUInt32());
			}
			return ""; // empty string if we don't recognize the type, should never happen
		}
	}

	[MsgPackSerializable(Layout.Indexed)]
	public sealed class CallbackInfoDelegate
	{
		[Index(0)] public string requestId { get; set; }
		[Index(1)] public string evName { get; set; }
		[Index(2)] public object[] args { get; set; }

		public CallbackInfoDelegate() { }
		public CallbackInfoDelegate(string requestId, string evName, object[] args)
		{
			this.requestId = requestId;
			this.evName = evName;
			this.args = args;
		}
	}
}
