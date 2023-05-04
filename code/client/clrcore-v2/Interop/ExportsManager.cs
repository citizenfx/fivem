using System;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using System.Runtime.CompilerServices;
using CitizenFX.Core.Native;

namespace CitizenFX.Core
{
	public delegate Coroutine<object> ExportFunc(params object[] args);

	internal static class ExportsManager
	{
		internal static string ExportPrefix { get; private set; }

		private static Dictionary<string, Tuple<DynFunc, Binding>> s_exports = new Dictionary<string, Tuple<DynFunc, Binding>>();

		internal static void Initialize(string resourceName)
		{
			ExportPrefix = CreateExportPrefix(resourceName);
		}

		internal static unsafe bool IncomingRequest(string eventName, string sourceString, Binding origin, byte* argsSerialized, int serializedSize, ref object[] args)
		{
			if (s_exports.TryGetValue(eventName, out var export) && (export.Item2 & origin) != 0)
			{
				if (args == null)
					args = MsgPackDeserializer.DeserializeArray(argsSerialized, serializedSize, origin == Binding.Remote ? sourceString : null);

				if (origin == Binding.Local)
				{
					// indirect invocation, return ref function identifier, see LocalInvoke() function below to get an idea
					if (args[0] is Callback cb)
						cb.Invoke(export.Item1);
				}
				else // REMOTE export request
				{
					if (args.Length > 3
						&& args[0] is string resource
						&& args[2] is object[] arguments
						&& Convert.ToUInt64(args[1]) is ulong replyId
						//&& Convert.ToUInt64(args[3]) is ulong timeout
						)
					{
						object result = null;

						try
						{
							result = export.Item1(new Remote(origin, sourceString), arguments);
						}
						catch (Exception ex)
						{
							if (Debug.ShouldWeLogDynFuncError(ex, export.Item1))
							{
								string argsString = string.Join<string>(", ", args.Select(a => a.GetType().ToString()));
								Debug.WriteLine($"^1Error while handling export: {eventName.Remove(0, eventName.LastIndexOf('_') + 1)}\n\twith arguments: ({argsString})^7");
								Debug.PrintError(ex);
							}
						}
#if IS_FXSERVER
						// last spot holds the player
						if (args[args.Length - 1] is Remote remote)
						{
							CString target = remote.GetPlayerHandle();
#else
						{
#endif
							if (result is Coroutine coroutine && !coroutine.IsCompleted)
							{
								if (!coroutine.IsCompleted)
								{
									coroutine.ContinueWith(() =>
									{
#if !IS_FXSERVER
										Events.TriggerServerEvent($"__cfx_rpcRep:{resource}", replyId, new[] { coroutine.GetResult(), coroutine.Exception?.ToString() });
#else
										CoreNatives.TriggerClientEventInternal($"__cfx_rpcRep:{resource}", target, new object[] { replyId, new[] { coroutine.GetResult(), coroutine.Exception?.ToString() } });
#endif
									});

									return true;
								}
								else
									result = coroutine.GetResult();
							}
#if !IS_FXSERVER
							Events.TriggerServerEvent($"__cfx_rpcRep:{resource}", replyId, new[] { result, null });
#else
							CoreNatives.TriggerClientEventInternal($"__cfx_rpcRep:{resource}", target, new object[] { replyId, new[] { result, null } });
						}
						else
						{
							Debug.WriteLine($"^1Error while replying to export call: {eventName.Remove(0, eventName.LastIndexOf('_') + 1)}\n\tCould not find recipient.^7");
#endif
						}
					}
				}

				return true;
			}

			return false;
		}

#if REMOTE_FUNCTION_ENABLED
#if IS_FXSERVER
		internal static Coroutine<object> RemoteInvoke(string resource, Player player, string export, params object[] args)
			=> RemoteInvoke(CreateFullExportName(resource, export), player, args);

		internal static Coroutine<object> RemoteInvoke(CString fullExportName, Player player, params object[] args)
#else
		internal static Promise<object> RemoteInvoke(string resource, string export, params object[] args)
			=> RemoteInvoke(CreateFullExportName(resource, export), args);

		internal static Promise<object> RemoteInvoke(CString fullExportName, params object[] args)
#endif
		{
			var promise = new Promise<object>();
			var callbackId = ExternalsManager.RegisterRemoteFunction(typeof(void), callbackArgs =>
			{
				if (callbackArgs.Length > 1 && callbackArgs[1] != null)
					promise.SetException(new Exception(callbackArgs[1].ToString()));
				else
					promise.Complete(callbackArgs[0]);

				return null;
			});

			ulong timeout = ulong.MaxValue;

#if IS_FXSERVER
			Events.TriggerClientEvent(fullExportName, player, ScriptInterface.CResourceName, callbackId, args, timeout);
#else
			Events.TriggerServerEvent(fullExportName, ScriptInterface.CResourceName, callbackId, args, timeout);
#endif
			return promise;
		}
#endif

		internal static Coroutine<object> LocalInvoke(string resource, string export, params object[] args)
			=> LocalInvoke(CreateFullExportName(resource, export), args);

		internal static Coroutine<object> LocalInvoke(CString fullExportName, params object[] args)
		{
			Callback callback = null;
			Events.TriggerEvent(fullExportName, new Action<Callback>(d => callback = d));

			return callback?.Invoke(args);
		}

		[SecuritySafeCritical]
		internal static bool AddExportHandler(string name, DynFunc method, Binding ports = Binding.Local)
		{
#if !REMOTE_FUNCTION_ENABLED
			if (ports == Binding.Remote)
			{
				Debug.WriteLine($"Remote exports aren't supported yet, ignoring registration of {name}");
				return false;
			}
#endif

			string eventName = CreateCurrentResourceFullExportName(name);
			if (!s_exports.ContainsKey(eventName))
			{
				s_exports.Add(eventName, new Tuple<DynFunc, Binding>(method, ports));
				CoreNatives.RegisterResourceAsEventHandler(eventName);

				return true;
			}

			return false;
		}

		internal static bool RemoveExportHandler(string name)
		{
			return s_exports.Remove(CreateCurrentResourceFullExportName(name));
		}


		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static string CreateFullExportName(string resource, string exportName) => "__cfx_export_" + resource + "_" + exportName;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static string CreateExportPrefix(string resource) => "__cfx_export_" + resource + "_";

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static string CreateCurrentResourceFullExportName(string exportName) => ExportPrefix + exportName;
	}
}
