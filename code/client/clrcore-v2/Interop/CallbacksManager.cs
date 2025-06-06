using CitizenFX.Core;
using CitizenFX.Core.Native;
using CitizenFX.MsgPack;
using CitizenFX.Shared;
using CitizenFX.Shared.Native;
using System;
using System.Collections.Concurrent;
using System.Security.Cryptography;
using System.Threading;

namespace CitizenFX.Core
{
	internal static class CallbacksManager
	{
		private static readonly ConcurrentDictionary<string, MsgPackFunc> _handlers = new ConcurrentDictionary<string, MsgPackFunc>();
		private static readonly ConcurrentDictionary<string, MsgPackFunc> _pending = new ConcurrentDictionary<string, MsgPackFunc>();
		private static int _requestCounter;
		private const string CallbackRequestEvent = "__cfx_cbrequest";
		private const string CallbackResponseEvent = "__cfx_cbresponse";
		private const int TimeoutMilliseconds = 5000; // too much? callbacks should never take > 1000ms
		internal delegate TResult TypeDeserializer<out TResult>(ref MsgPackDeserializer arg);

		static CallbacksManager()
		{
			Native.CoreNatives.RegisterResourceAsEventHandler(CallbackResponseEvent);
			Native.CoreNatives.RegisterResourceAsEventHandler(CallbackRequestEvent);
		}

		internal static unsafe bool IncomingRequest(string eventName, string sourceString, Binding origin, byte* argsSerialized, int serializedSize)
		{
			Remote remote = new Remote(origin, sourceString);
			CitizenFX.MsgPack.MsgPackDeserializer deserializer = new MsgPackDeserializer(argsSerialized, (ulong)serializedSize, origin == Binding.Remote ? sourceString : null);
			var restorePoint = deserializer.CreateRestorePoint();

			if (eventName == CallbackResponseEvent)
			{
				uint length = deserializer.ReadArraySize();
				string requestId = ReadStringBytes(ref deserializer);
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

							if (result is Coroutine<object> coro)
							{
								coro.ContinueWith(r =>
								{
									Send(CallbackResponseEvent, playerHandle, origin,requestId,r.GetResult());
								});
							}
							else
							{
								Send(CallbackResponseEvent,playerHandle,origin,requestId,((Coroutine)result).GetResultNonThrowing());
							}
						}
					}
					catch (Exception ex)
					{

						Debug.WriteException(ex, handler, callbackInfo.args, "callbacks manager");
						//Send(CallbackResponseEvent,playerHandle,origin,requestId,null);
					}
				}
				else
				{
					Send(CallbackResponseEvent,playerHandle,Binding.Remote,requestId,null);
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

		internal static void Register<T>(string name, Binding binding, MsgPackFunc handler)
		{
			if(!_handlers.TryAdd(name, (Remote remote, ref MsgPackDeserializer deserializer) =>
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

#if IS_FXSERVER
		internal static Coroutine<T> TriggerClientCallback<T>(Player client, string name, params object[] args)
		{
			return TriggerInternal<T>(Binding.Remote, client.m_handle, name, args);
		}
#endif
		internal static Coroutine<T> TriggerServerCallback<T>(string name, params object[] args)
		{
			return TriggerInternal<T>(Binding.Remote, null, name, args);
		}
		internal static Coroutine<T> TriggerLocalCallback<T>(string name, params object[] args)
		{
			return TriggerInternal<T>(Binding.Local, null, name, args);
		}

		internal static Coroutine<T> TriggerInternal<T>(Binding binding, string target, string name, params object[] args)
		{
			string requestId = GenerateRequestId();
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
					coro.Fail(null, new TimeoutException($"Callback '{name}' timed out after {TimeoutMilliseconds}ms."));
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

			Send(eventName, target, binding, del);
		}

		private static void Send(string eventName, string target, Binding binding, string requestId, object result)
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

		private static void Send(string eventName, string target, Binding binding, CallbackInfoDelegate del)
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

		private static string GenerateRequestId()
		{
			int id = Interlocked.Increment(ref _requestCounter);
#if IS_FXSERVER
			return "srv_" + id;
#else
			return "cli_" + id;
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
