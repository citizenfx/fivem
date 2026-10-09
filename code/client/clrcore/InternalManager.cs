using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Linq.Expressions;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	class InternalManager : MarshalByRefObject, InternalManagerInterface
	{
		private static readonly List<BaseScript> ms_definedScripts = new List<BaseScript>();
		private static readonly List<Tuple<DateTime, AsyncCallback, string>> ms_delays = new List<Tuple<DateTime, AsyncCallback, string>>();
		private static int ms_instanceId;
		private static bool ms_useSyncContext;

		private string m_resourceName;

		public static IScriptHost ScriptHost { get; internal set; }

		// actually, domain-global
		internal static InternalManager GlobalManager { get; set; }

		internal string ResourceName => m_resourceName;

		[SecuritySafeCritical]
		public InternalManager()
		{
			//CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
			//CultureInfo.DefaultThreadCurrentUICulture = CultureInfo.InvariantCulture;

			GlobalManager = this;

			Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
			Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

			InitializeAssemblyResolver();

			CitizenTaskScheduler.Create();
		}

		[SecuritySafeCritical]
		public void CreateTaskScheduler()
		{
			CitizenTaskScheduler.MakeDefault();

#if !IS_FXSERVER
			SynchronizationContext.SetSynchronizationContext(new CitizenSynchronizationContext());
#endif
		}

		[SecuritySafeCritical]
		public void Destroy()
		{
			if (m_retvalBuffer != IntPtr.Zero)
			{
				Marshal.FreeHGlobal(m_retvalBuffer);
				m_retvalBuffer = IntPtr.Zero;
			}
		}

		public void SetResourceName(string resourceName)
		{
			m_resourceName = resourceName;
		}

		public void SetConfiguration(bool useSyncContext)
		{
			ms_useSyncContext = useSyncContext;
		}

		[SecuritySafeCritical]
		public void SetScriptHost(IScriptHost host, int instanceId)
		{
			ScriptHost = host;
			ms_instanceId = instanceId;
		}

		[SecuritySafeCritical]
		public void SetScriptHost(IntPtr hostPtr, int instanceId)
		{
			ScriptHost = new DirectScriptHost(hostPtr);
			ms_instanceId = instanceId;
		}

		internal static void AddScript(BaseScript script)
		{
			if (!ms_definedScripts.Contains(script))
			{
				script.InitializeOnAdd();

				ms_definedScripts.Add(script);
			}
		}

		internal static void RemoveScript(BaseScript script)
		{
			if (ms_definedScripts.Contains(script))
			{
				ms_definedScripts.Remove(script);
			}
		}

		public void CreateAssembly(string scriptFile, byte[] assemblyData, byte[] symbolData)
		{
			CreateAssemblyInternal(scriptFile, assemblyData, symbolData);
		}

		static Dictionary<string, Assembly> ms_loadedAssemblies = new Dictionary<string, Assembly>();

		[SecuritySafeCritical]
		private static Assembly CreateAssemblyInternal(string assemblyFile, byte[] assemblyData, byte[] symbolData)
		{
			if (ms_loadedAssemblies.ContainsKey(assemblyFile))
			{
#if !IS_FXSERVER
				Debug.WriteLine("Returning previously loaded assembly {0}", ms_loadedAssemblies[assemblyFile].FullName);
#endif
				return ms_loadedAssemblies[assemblyFile];
			}

			var assembly = Assembly.Load(assemblyData, symbolData);
#if !IS_FXSERVER
			Debug.WriteLine("Loaded {1} into {0}", AppDomain.CurrentDomain.FriendlyName, assembly.FullName);
#endif

			ms_loadedAssemblies[assemblyFile] = assembly;

			Func<Type, bool> typesPredicate = t =>
				t != null && !t.IsAbstract && t.IsSubclassOf(typeof(BaseScript)) && t.GetConstructor(Type.EmptyTypes) != null;

			// We have ClientScript and ServerScript defined only in the respective environments.
			// Handle type load exceptions and keep going.
			// See https://stackoverflow.com/a/11915414
			IEnumerable<Type> definedTypes;
			try
			{
				definedTypes = assembly.GetTypes().Where(typesPredicate);
			}
			catch (ReflectionTypeLoadException e)
			{
				definedTypes = e.Types.Where(typesPredicate);
			}

			foreach (var type in definedTypes)
			{
				try
				{
					var derivedScript = Activator.CreateInstance(type) as BaseScript;
					
					Debug.WriteLine("Instantiated instance of script {0}.", type.FullName);

					AddScript(derivedScript);
				}
				catch (Exception e)
				{
					Debug.WriteLine("Failed to instantiate instance of script {0}: {1}", type.FullName, e.ToString());
				}
			}

			return assembly;
		}

		[SecuritySafeCritical]
		void InitializeAssemblyResolver()
		{
			AppDomain.CurrentDomain.AssemblyResolve += CurrentDomain_AssemblyResolve;

			AppDomain.CurrentDomain.UnhandledException += CurrentDomain_UnhandledException;
		}

		static void CurrentDomain_UnhandledException(object sender, UnhandledExceptionEventArgs e)
		{
			Debug.WriteLine("Unhandled exception: {0}", e.ExceptionObject.ToString());
		}

		private static HashSet<string> ms_assemblySearchPaths = new HashSet<string>();

		public void LoadAssembly(string name)
		{
			LoadAssemblyInternal(name.Replace(".dll", ""));
		}

		static Assembly LoadAssemblyInternal(string baseName, bool useSearchPaths = false)
		{
			var attemptPaths = new List<string>();
			attemptPaths.Add(baseName);

			var exceptions = new StringBuilder();

			if (useSearchPaths)
			{
				foreach (var path in ms_assemblySearchPaths)
				{
					attemptPaths.Add($"{path.Replace('\\', '/')}/{baseName}");
				}
			}

			foreach (var name in attemptPaths)
			{
				try
				{
					byte[] assemblyBytes;

					using (var assemblyStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".dll"))))
					{
						assemblyBytes = assemblyStream.ReadBytes((int)assemblyStream.BaseStream.Length);
					}

					byte[] symbolBytes = null;

					try
					{
						using (var symbolStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".dll.mdb"))))
						{
							symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
						}
					}
					catch
					{
						try
						{
							using (var symbolStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".pdb"))))
							{
								symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
							}
						}
						catch
						{
							// nothing
						}
					}

					if (assemblyBytes != null)
					{
						var dirName = Path.GetDirectoryName(name);

						if (!string.IsNullOrWhiteSpace(dirName))
						{
							ms_assemblySearchPaths.Add(dirName);
						}
					}

					return CreateAssemblyInternal(name + ".dll", assemblyBytes, symbolBytes);
				}
				catch (Exception e)
				{
					//Switching the FileNotFound to a NotImplemented tells mono to disable I18N support.
					//See: https://github.com/mono/mono/blob/8fee89e530eb3636325306c66603ba826319e8c5/mcs/class/corlib/System.Text/EncodingHelper.cs#L131
					if (e is FileNotFoundException && string.Equals(name, "I18N", StringComparison.OrdinalIgnoreCase))
						throw new NotImplementedException("I18N not found", e);

					exceptions.AppendLine($"Exception loading assembly {name}: {e}");
				}
			}

			if (!baseName.Contains(".resources"))
			{
				Debug.WriteLine($"Could not load assembly {baseName} - loading exceptions: {exceptions}");
			}

			return null;
		}

		[SecuritySafeCritical]
		public byte[] WalkStack(byte[] boundaryStart, byte[] boundaryEnd)
		{
			var success = GameInterface.WalkStackBoundary(m_resourceName, boundaryStart, boundaryEnd, out var blob);

			return (success) ? blob : null;
		}

		static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
		{
			return LoadAssemblyInternal(args.Name.Split(',')[0], useSearchPaths: true);
		}

		public static void AddDelay(int delay, AsyncCallback callback, string name = null)
		{
			lock (ms_delays)
				ms_delays.Add(Tuple.Create(DateTime.UtcNow.AddMilliseconds(delay), callback, name));
		}

		public static void TickGlobal()
		{
			GlobalManager.Tick();
		}

		[SecuritySafeCritical]
		public void Tick()
		{
			IsProfiling = ProfilerIsRecording();

			if (GameInterface.SnapshotStackBoundary(out var b))
			{
				ScriptHost.SubmitBoundaryStart(b, b.Length);
			}

			try
			{
				using (var scope = new ProfilerScope(() => "c# cleanup"))
				{
					ScriptContext.GlobalCleanUp();
				}

				using (var scope = new ProfilerScope(() => "c# deferredDelay"))
				{
					var now = DateTime.UtcNow;
					lock (ms_delays)
					{
						var count = ms_delays.Count;

						for (int delayIdx = 0; delayIdx < count; delayIdx++)
						{
							var delay = ms_delays[delayIdx];

							if (now >= delay.Item1)
							{
								using (var inScope = new ProfilerScope(() => delay.Item3))
								{
									try
									{
										BaseScript.CurrentName = delay.Item3;
										delay.Item2(new DummyAsyncResult());
									}
									finally
									{
										BaseScript.CurrentName = null;
									}
								}

								ms_delays.RemoveAt(delayIdx);
								delayIdx--;
								count--;
							}
						}
					}
				}

				using (var scope = new ProfilerScope(() => "c# schedule"))
				{
					var flowBlock = CitizenTaskScheduler.SuppressFlow();

					foreach (var script in ms_definedScripts.ToArray())
					{
						script.ScheduleRun();
					}

					flowBlock?.Undo();
				}

				using (var scope = new ProfilerScope(() => "c# tasks"))
				{
					CitizenTaskScheduler.Instance.Tick();
				}

				using (var scope = new ProfilerScope(() => "c# syncCtx"))
				{
					CitizenSynchronizationContext.Tick();
				}
			}
			catch (Exception e)
			{
				PrintError("tick", e);
			}
			finally
			{
				ScriptHost.SubmitBoundaryStart(null, 0);
			}
		}

		private class SyncContextScope : IDisposable
		{
			private static SynchronizationContext citizenContext = new CitizenSynchronizationContext();
			private SynchronizationContext oldContext;

			public SyncContextScope()
			{
				if (ms_useSyncContext)
				{
					oldContext = SynchronizationContext.Current;
					SynchronizationContext.SetSynchronizationContext(citizenContext);
				}
			}

			public void Dispose()
			{
				if (oldContext != null)
				{
					SynchronizationContext.SetSynchronizationContext(oldContext);
				}
			}
		}

		[SecuritySafeCritical]
		public void TriggerEvent(string eventName, byte[] argsSerialized, string sourceString)
		{
			// not using using statements here as old Mono on Linux build doesn't know of these
#if IS_FXSERVER
			using (var syncContext = new SyncContextScope())
#endif
			{
				if (GameInterface.SnapshotStackBoundary(out var bo))
				{
					ScriptHost.SubmitBoundaryStart(bo, bo.Length);
				}

				try
				{
#if IS_FXSERVER
					var netSource = (sourceString.StartsWith("net") || sourceString.StartsWith("internal-net")) ? sourceString : null;
#else
					var netSource = sourceString.StartsWith("net") ? sourceString : null;
#endif

					var obj = MsgPackDeserializer.Deserialize(argsSerialized, netSource) as List<object> ?? (IEnumerable<object>)new object[0];

					var scripts = ms_definedScripts.ToArray();

					var objArray = obj.ToArray();

					NetworkFunctionManager.HandleEventTrigger(eventName, objArray, sourceString);

					foreach (var script in scripts)
					{
						Task.Factory.StartNew(() =>
						{
							BaseScript.CurrentName = $"eventHandler {script.GetType().Name} -> {eventName}";
							var t = script.EventHandlers.Invoke(eventName, sourceString, objArray);
							BaseScript.CurrentName = null;

							return t;
						}, CancellationToken.None, TaskCreationOptions.None, CitizenTaskScheduler.Instance).Unwrap().ContinueWith(a =>
						{
							if (a.IsFaulted)
							{
								Debug.WriteLine($"Error invoking event handlers for {eventName}: {a.Exception?.InnerExceptions.Aggregate("", (b, s) => s + b.ToString() + "\n")}");
							}
						});
					}

					ExportDictionary.Invoke(eventName, objArray);

					// invoke a single task tick
					CitizenTaskScheduler.Instance.Tick();
				}
				catch (Exception e)
				{
					PrintError($"event ({eventName})", e);
				}
				finally
				{
					ScriptHost.SubmitBoundaryStart(null, 0);
				}
			}
		}

		[SecuritySafeCritical]
		public static string CanonicalizeRef(int refId)
		{
			var re = ScriptHost.CanonicalizeRef(refId, ms_instanceId);
			var str = Marshal.PtrToStringAnsi(re);

			GameInterface.fwFree(re);

			return str;
		}

		private IntPtr m_retvalBuffer;
		private int m_retvalBufferSize;

		[SecuritySafeCritical]
		public void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retval)
		{
			// not using using statements here as old Mono on Linux build doesn't know of these
#if IS_FXSERVER
			using (var syncContext = new SyncContextScope())
#endif
			{

				if (GameInterface.SnapshotStackBoundary(out var b))
				{
					ScriptHost.SubmitBoundaryStart(b, b.Length);
				}

				try
				{
					var retvalData = FunctionReference.Invoke(refIndex, argsSerialized);

					if (retvalData != null)
					{
						retval = GameInterface.MakeMemoryBuffer(retvalData);
					}
					else
					{
						retval = IntPtr.Zero;
					}
				}
				catch (Exception e)
				{
					var formattedStackTrace = CreateStackTrace("reference call", e.InnerException ?? e);
					var err = MsgPackSerializer.Serialize(new object[] { false, formattedStackTrace });

					retval = GameInterface.MakeMemoryBuffer(err);
				}
			}
		}

		public int DuplicateRef(int refIndex)
		{
			return FunctionReference.Duplicate(refIndex);
		}

		public void RemoveRef(int refIndex)
		{
			FunctionReference.Remove(refIndex);
		}

		[SecuritySafeCritical]
		public ulong GetMemoryUsage()
		{
			return GameInterface.GetMemoryUsage();
		}

		[SecuritySafeCritical]
		public static T CreateInstance<T>(Guid clsid)
		{
			var hr = GameInterface.CreateObjectInstance(clsid, typeof(T).GUID, out IntPtr ptr);

			if (hr < 0)
			{
				Marshal.ThrowExceptionForHR(hr);
			}

			var obj = (T)Marshal.GetObjectForIUnknown(ptr);
			Marshal.Release(ptr);

			return obj;
		}

		[SecurityCritical]
		public override object InitializeLifetimeService()
		{
			return null;
		}

		internal static void PrintErrorInternal(string where, Exception what)
		{
			GlobalManager?.PrintError(where, what);
		}

		[SecuritySafeCritical]
		private void PrintError(string where, Exception what)
		{
			var formattedStackTrace = CreateStackTrace(where, what);

			if (formattedStackTrace != null)
			{
				Debug.WriteLine($"^1SCRIPT ERROR in {where}: {what.GetType().FullName}: {what.Message}^7");
				Debug.WriteLine("{0}", formattedStackTrace);
			}
		}

		private string CreateStackTrace(string where, Exception what)
		{
			ScriptHost.SubmitBoundaryEnd(null, 0);

			var stackTrace = new StackTrace(what, true);

#if IS_FXSERVER
			var stackFrames = stackTrace.GetFrames();
#else
			IEnumerable<StackFrame> stackFrames;

			// HACK: workaround to iterate inner traces ourselves.
			// TODO: remove this once we've updated libraries
			var fieldCapturedTraces = typeof(StackTrace).GetField("captured_traces", BindingFlags.NonPublic | BindingFlags.Instance);
			if (fieldCapturedTraces != null)
			{
				var captured_traces = (StackTrace[])fieldCapturedTraces.GetValue(stackTrace);

				// client's mscorlib is missing this piece of code, copied from https://github.com/mono/mono/blob/ef848cfa83ea16b8afbd5b933968b1838df19505/mcs/class/corlib/System.Diagnostics/StackTrace.cs#L181
				var accum = new List<StackFrame>();
				foreach (var t in captured_traces ?? Array.Empty<StackTrace>())
				{
					for (int i = 0; i < t.FrameCount; i++)
						accum.Add(t.GetFrame(i));
				}

				accum.AddRange(stackTrace.GetFrames());

				stackFrames = accum;
			}
			else
				stackFrames = stackTrace.GetFrames();
#endif

			var frames = stackFrames
				.Select(a => new
				{
					Frame = a,
					Method = a.GetMethod(),
					Type = a.GetMethod()?.DeclaringType
				})
				.Where(a => a.Method != null && (!a.Type.Assembly.GetName().Name.Contains("mscorlib") && !a.Type.Assembly.GetName().Name.Contains("CitizenFX.Core") && !a.Type.Assembly.GetName().Name.StartsWith("System")))
				.Select(a => new
				{
					name = EnhancedStackTrace.GetMethodDisplayString(a.Method).ToString(),
					sourcefile = a.Frame.GetFileName() ?? "",
					line = a.Frame.GetFileLineNumber(),
					file = $"@{m_resourceName}/{a.Type?.Assembly.GetName().Name ?? "UNK"}.dll"
				});

			var serializedFrames = MsgPackSerializer.Serialize(frames);
			var formattedStackTrace = FormatStackTrace(serializedFrames);

			return formattedStackTrace;
		}

		[SecurityCritical]
		private unsafe string FormatStackTrace(byte[] serializedFrames)
		{
			fixed (byte* ptr = serializedFrames)
			{
				return Native.Function.Call<string>((Native.Hash)0xd70c3bca, ptr, serializedFrames.Length);
			}
		}

		private class DirectScriptHost : IScriptHost
		{
			private IntPtr hostPtr;

			private FastMethod<Func<IntPtr, IntPtr, int>> invokeNativeMethod;
			private FastMethod<Func<IntPtr, IntPtr, IntPtr, int>> openSystemFileMethod;
			private FastMethod<Func<IntPtr, IntPtr, IntPtr, int>> openHostFileMethod;
			private FastMethod<Func<IntPtr, int, int, IntPtr, int>> canonicalizeRefMethod;
			private FastMethod<Action<IntPtr, IntPtr>> scriptTraceMethod;
			private FastMethod<Action<IntPtr, IntPtr, int>> submitBoundaryStartMethod;
			private FastMethod<Action<IntPtr, IntPtr, int>> submitBoundaryEndMethod;
			private FastMethod<Func<IntPtr, IntPtr, int>> getLastErrorTextMethod;
			private FastMethod<Func<IntPtr, IntPtr, IntPtr, int, IntPtr, int>> invokeFunctionReference;

			[SecuritySafeCritical]
			public DirectScriptHost(IntPtr hostPtr)
			{
				this.hostPtr = hostPtr;

				invokeNativeMethod = new FastMethod<Func<IntPtr, IntPtr, int>>(nameof(invokeNativeMethod), hostPtr, 0);
				openSystemFileMethod = new FastMethod<Func<IntPtr, IntPtr, IntPtr, int>>(nameof(openSystemFileMethod), hostPtr, 1);
				openHostFileMethod = new FastMethod<Func<IntPtr, IntPtr, IntPtr, int>>(nameof(openHostFileMethod), hostPtr, 2);
				canonicalizeRefMethod = new FastMethod<Func<IntPtr, int, int, IntPtr, int>>(nameof(canonicalizeRefMethod), hostPtr, 3);
				scriptTraceMethod = new FastMethod<Action<IntPtr, IntPtr>>(nameof(scriptTraceMethod), hostPtr, 4);
				submitBoundaryStartMethod = new FastMethod<Action<IntPtr, IntPtr, int>>(nameof(submitBoundaryStartMethod), hostPtr, 5);
				submitBoundaryEndMethod = new FastMethod<Action<IntPtr, IntPtr, int>>(nameof(submitBoundaryEndMethod), hostPtr, 6);
				getLastErrorTextMethod = new FastMethod<Func<IntPtr, IntPtr, int>>(nameof(getLastErrorTextMethod), hostPtr, 7);
				invokeFunctionReference = new FastMethod<Func<IntPtr, IntPtr, IntPtr, int, IntPtr, int>>(nameof(invokeFunctionReference), hostPtr, 8);
			}

			[SecuritySafeCritical]
			public void InvokeNative([MarshalAs(UnmanagedType.Struct)] IntPtr context)
			{
				var hr = invokeNativeMethod.method(hostPtr, context);
				Marshal.ThrowExceptionForHR(hr);
			}

			[SecuritySafeCritical]
			public fxIStream OpenSystemFile(string fileName)
			{
				return OpenSystemFileInternal(fileName);
			}

			[SecurityCritical]
			private unsafe fxIStream OpenSystemFileInternal(string fileName)
			{
				IntPtr retVal = IntPtr.Zero;

				IntPtr stringRef = Marshal.StringToHGlobalAnsi(fileName);

				try
				{
					IntPtr* retValRef = &retVal;

					Marshal.ThrowExceptionForHR(openSystemFileMethod.method(hostPtr, stringRef, new IntPtr(retValRef)));
				}
				finally
				{
					Marshal.FreeHGlobal(stringRef);
				}

				var s = (fxIStream)Marshal.GetObjectForIUnknown(retVal);
				Marshal.Release(retVal);

				return s;
			}

			[SecuritySafeCritical]
			public fxIStream OpenHostFile(string fileName)
			{
				return OpenHostFileInternal(fileName);
			}

			[SecurityCritical]
			private unsafe fxIStream OpenHostFileInternal(string fileName)
			{
				IntPtr retVal = IntPtr.Zero;

				IntPtr stringRef = Marshal.StringToHGlobalAnsi(fileName);

				try
				{
					IntPtr* retValRef = &retVal;

					Marshal.ThrowExceptionForHR(openHostFileMethod.method(hostPtr, stringRef, new IntPtr(retValRef)));
				}
				finally
				{
					Marshal.FreeHGlobal(stringRef);
				}

				var s = (fxIStream)Marshal.GetObjectForIUnknown(retVal);
				Marshal.Release(retVal);

				return s;
			}

			[SecuritySafeCritical]
			public IntPtr CanonicalizeRef(int localRef, int instanceId)
			{
				return CanonicalizeRefInternal(localRef, instanceId);
			}

			[SecurityCritical]
			private unsafe IntPtr CanonicalizeRefInternal(int localRef, int instanceId)
			{
				IntPtr retVal = IntPtr.Zero;

				try
				{
					IntPtr* retValRef = &retVal;

					Marshal.ThrowExceptionForHR(canonicalizeRefMethod.method(hostPtr, localRef, instanceId, new IntPtr(retValRef)));
				}
				finally
				{

				}

				return retVal;
			}

			[SecuritySafeCritical]
			public void ScriptTrace([MarshalAs(UnmanagedType.LPStr)] string message)
			{
				ScriptTraceInternal(message);
			}

			[SecurityCritical]
			private unsafe void ScriptTraceInternal(string message)
			{
				var bytes = Encoding.UTF8.GetBytes(message);

				fixed (byte* p = bytes)
				{
					scriptTraceMethod.method(hostPtr, new IntPtr(p));
				}
			}

			[SecuritySafeCritical]
			public void SubmitBoundaryStart(byte[] boundaryData, int boundarySize)
			{
				SubmitBoundaryInternal(submitBoundaryStartMethod, boundaryData, boundarySize);
			}

			[SecuritySafeCritical]
			public void SubmitBoundaryEnd(byte[] boundaryData, int boundarySize)
			{
				SubmitBoundaryInternal(submitBoundaryEndMethod, boundaryData, boundarySize);
			}

			[SecurityCritical]
			private unsafe void SubmitBoundaryInternal(FastMethod<Action<IntPtr, IntPtr, int>> method, byte[] boundaryData, int boundarySize)
			{
				fixed (byte* p = boundaryData)
				{
					method.method(hostPtr, new IntPtr(p), boundarySize);
				}
			}

			[SecuritySafeCritical]
			public IntPtr GetLastErrorText()
			{
				return GetLastErrorTextInternal();
			}

			[SecurityCritical]
			private unsafe IntPtr GetLastErrorTextInternal()
			{
				IntPtr retVal = IntPtr.Zero;

				try
				{
					IntPtr* retValRef = &retVal;

					Marshal.ThrowExceptionForHR(getLastErrorTextMethod.method(hostPtr, new IntPtr(retValRef)));
				}
				finally
				{

				}

				return retVal;
			}

			public unsafe int InvokeFunctionReference(string refId, byte[] argsSerialized, int argsSize, IntPtr ret)
			{
				var refIdBytes = Encoding.UTF8.GetBytes(refId);

				fixed (byte* refIdBytesFixed = refIdBytes)
				{
					fixed (byte* argsSerializedStart = argsSerialized)
					{
						return invokeFunctionReference.method(hostPtr, new IntPtr(refIdBytesFixed), new IntPtr(argsSerializedStart), argsSize, ret);
					}
				}
			}
		}

		internal static bool ProfilerIsRecording()
		{
			return Native.API.ProfilerIsRecording();
		}

		internal static void ProfilerEnterScope(string scopeName)
		{
			Native.API.ProfilerEnterScope(scopeName);
		}

		internal static void ProfilerExitScope()
		{
			Native.API.ProfilerExitScope();
		}

		internal static bool IsProfiling { get; private set; }
	}


	internal class ProfilerScope : IDisposable
	{
		public ProfilerScope(Func<string> name)
		{
			if (!InternalManager.IsProfiling)
			{
				return;
			}

			InternalManager.ProfilerEnterScope(name() ?? "c# scope");
		}

		public void Dispose()
		{
			if (!InternalManager.IsProfiling)
			{
				return;
			}

			InternalManager.ProfilerExitScope();
		}
	}
}
