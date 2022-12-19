using System;
using System.ComponentModel;
using System.Collections.Generic;
using System.Linq;
using System.Security;
using CitizenFX.Core.Native;
using System.Runtime.CompilerServices;

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
					args = MsgPackDeserializer.DeserializeArguments(argsSerialized, serializedSize, sourceString, true);

				if (origin == Binding.LOCAL)
				{
					// indirect invocation, return ref function identifier, see LocalInvoke() function below to get an idea
					if(args[0] is Callback cb)
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
							result = export.Item1(arguments);
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
						string target = (args[args.Length - 1] as Player).Handle; // last spot holds the player
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
#endif
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
		internal static bool AddExportHandler(string name, DynFunc method, Binding ports = Binding.LOCAL)
		{
#if !REMOTE_FUNCTION_ENABLED
			if (ports == Binding.REMOTE)
			{
				Debug.WriteLine($"Remote exports aren't supported yet, ignoring registration of {name}");
				return false;
			}
#endif

			string eventName = CreateCurrentResourceFullExportName(name);
			if (!s_exports.ContainsKey(eventName))
			{
				s_exports.Add(eventName, new Tuple<DynFunc, Binding>(method, ports));
				Native.CoreNatives.RegisterResourceAsEventHandler(eventName);

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

	public class Exports
	{
		private Dictionary<string, DynFunc> m_exports => new Dictionary<string, DynFunc>();

		public static LocalExports Local { get; } = new LocalExports();

#if REMOTE_FUNCTION_ENABLED
#if !IS_FXSERVER
		public static RemoteExports Remote { get; } = new RemoteExports();
		public static RemoteExports Server { get; } = new RemoteExports();
#else
		public static RemoteExports Remote { get; } = new RemoteExports();
		public static RemoteExports Client { get; } = new RemoteExports();

		public ExportFunc this[Player player, string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, player, args);
			}
		}
#endif
#endif

		~Exports()
		{
			foreach(var export in m_exports.Keys)
				ExportsManager.RemoveExportHandler(export);
		}

		public ExportFunc this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}

		public DynFunc this[string export]
		{
			set => Add(export, value);
			get => m_exports[export];
		}

#if REMOTE_FUNCTION_ENABLED
		public DynFunc this[string export, Binding binding = Binding.LOCAL]
		{
			set => Add(export, value, binding);
		}
#endif

		public void Add(string name, DynFunc method, Binding binding = Binding.LOCAL)
		{
			if (ExportsManager.AddExportHandler(name, method, binding))
				m_exports.Add(name, method);
		}

		public void Remove(string name)
		{
			if (m_exports.Remove(name))
				ExportsManager.RemoveExportHandler(name);
		}
	}

	public class ResourceExports
	{
		public string resourcePrefix;

		internal ResourceExports(string name)
		{
			resourcePrefix = ExportsManager.CreateExportPrefix(name);
		}

		public ExportFunc this[string exportName]
		{
			get
			{
				CString fullExportName = resourcePrefix + exportName;
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}
	}

	[Browsable(false)]
	[EditorBrowsable(EditorBrowsableState.Never)]
	public struct LocalExports
	{
		public ExportFunc this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateFullExportName(resource, export);
				return (args) => ExportsManager.LocalInvoke(fullExportName, args);
			}
		}
	}

#if REMOTE_FUNCTION_ENABLED
	[Browsable(false)]
	[EditorBrowsable(EditorBrowsableState.Never)]
	public struct RemoteExports
	{
#if IS_FXSERVER
		public Callback this[Player player, string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, export, args);
			}
		}
#else
		public Callback this[string resource, string export]
		{
			get
			{
				CString fullExportName = ExportsManager.CreateExportName(resource, export);
				return (args) => ExportManager.RemoteInvoke(fullExportName, args);
			}
		}
#endif
	}
#endif
}
