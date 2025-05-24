#if REMOTE_FUNCTION_ENABLED
using CitizenFX.Core.Native;
using CitizenFX.MsgPack;
using System;
using System.Collections.Generic;
using System.Linq;

namespace CitizenFX.Core
{
	static class ExternalsManager
	{
		internal static string RpcRequestName { get; private set; }
		internal static string RpcReplyName { get; private set; }

		private static Dictionary<int, DynFunc> s_remoteFuncHandlers = new Dictionary<int, DynFunc>();
#if IS_FXSERVER
		private static Dictionary<string, HashSet<int>> s_playeRemoteHandlers = new Dictionary<string, HashSet<int>>();
#endif

		internal static void Initialize(string resourceName, int instanceId)
		{
			RpcRequestName = "__cfx_rpcReq:"+ resourceName;
			RpcReplyName = "__cfx_rpcRep:" + resourceName;

			Native.CoreNatives.RegisterResourceAsEventHandler(RpcRequestName);
			Native.CoreNatives.RegisterResourceAsEventHandler(RpcReplyName);

#if IS_FXSERVER
			EventManager.AddEventHandler("playerDropped", new DynFunc(OnPlayerDropped));
#endif
		}

		internal static unsafe bool IncomingRequest(string eventName, string sourceString, Binding origin, byte* argsSerialized, int serializedSize)
		{
			// only accepted for remote, might change in the future
			if (origin == Binding.REMOTE)
			{
				if (eventName == RpcRequestName)
				{
					var args = MsgPackDeserializer.DeserializeArguments(argsSerialized, serializedSize, origin == Binding.Remote ? sourceString : null);
					if (args.Length > 3 && args[2] is string requestId)
					{
						ulong rid = Convert.ToUInt64(requestId);

						int id = (int)rid;

						if (s_remoteFuncHandlers.TryGetValue(id, out var func)
							&& args[3] is object[] requestArguments
							&& args[1] is ulong replyId
							&& args[0] is string resource)
						{
							object result = null;

							try
							{
								result = func(requestArguments);
							}
							catch (Exception ex)
							{
								if (Debug.ShouldWeLogDynFuncError(ex, func))
								{
									string argsString = string.Join<string>(", ", args.Select(a => a.GetType().ToString()));
									Debug.WriteLine($"^1Error while handling RPC request\n\twith arguments: ({argsString})^7");
									Debug.PrintError(ex);
								}
							}

							if (replyId != ulong.MaxValue) // requesting reply
							{
#if !IS_FXSERVER
								Events.TriggerServerEvent($"__cfx_rpcRep:{resource}", replyId, new[] { result });
#else
								string player = (args[args.Length - 1] as Player).Handle;
								CoreNatives.TriggerClientEventInternal($"__cfx_rpcRep:{resource}", player, new object[] { replyId, new[] { result } });

								if (s_playeRemoteHandlers.TryGetValue(player, out var set))
								{
									set.Remove(id);
								}
#endif
							}
						}
					}

					return true;
				}
				else if (eventName == RpcReplyName)
				{
					args = MsgPackDeserializer.DeserializeArguments(argsSerialized, serializedSize, sourceString, true);
					if (args.Length > 1)
					{
						ulong replyId = (args[0] is string s) ? ulong.Parse(s) : Convert.ToUInt64(args[0]);
						int id = (int)replyId;

						if (s_remoteFuncHandlers.TryGetValue(id, out var func)
							&& args[1] is object[] callbackArguments)
						{
							s_remoteFuncHandlers.Remove(id);

							try
							{
								func(callbackArguments);
							}
							catch (Exception ex)
							{
								if (Debug.ShouldWeLogDynFuncError(ex, func))
								{
									string argsString = string.Join<string>(", ", args.Select(a => a.GetType().ToString()));
									Debug.WriteLine($"^1Error while handling RPC reply\n\twith arguments: ({argsString})^7");
									Debug.PrintError(ex);
								}
							}
						}
					}

					return true;
				}
			}

			return false;
		}

		internal static ulong RegisterRemoteFunction(Type returnType, DynFunc deleg)
		{
			int id = deleg.GetHashCode();
			if (deleg.Target != null)
				id ^= deleg.Target.GetHashCode();

			// keep incrementing until we find a free spot
			while (s_remoteFuncHandlers.ContainsKey(id)) unchecked { ++id; }
			s_remoteFuncHandlers.Add(id, deleg);

			ulong callbackId = (uint)id;
			if (returnType != typeof(void))
				callbackId |= (ulong)_ExternalFunction.Flags.HAS_RETURN_VALUE; // any type

			return callbackId;
		}

		internal static void UnRegisterRemoteFunction(ulong id)
		{
			s_remoteFuncHandlers.Remove((int)id);
		}

#if IS_FXSERVER
		internal static object OnPlayerDropped(object[] args)
		{
			try
			{
				// last argument is the Player
				Player player = (Player)args[args.Length - 1];

				if (s_playeRemoteHandlers.TryGetValue(player.Handle, out var set))
				{
					s_playeRemoteHandlers.Remove(player.Handle);

					foreach (var entry in set)
					{
						s_remoteFuncHandlers.Remove(entry);
					}
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine($"RPC playerDropped exception: {e}");
			}

			return null;
		}

		internal static void AddRPCToPlayer(string source, int callbackId)
		{
			// add a player promise
			if (!s_playeRemoteHandlers.ContainsKey(source))
			{
				s_playeRemoteHandlers[source] = new HashSet<int>();
			}

			s_playeRemoteHandlers[source].Add(callbackId);
		}
#endif
	}
}
#endif
