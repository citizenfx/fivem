using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Runtime.InteropServices;
using System.Security;
using System.Threading;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	class InternalManager : MarshalByRefObject, InternalManagerInterface
	{
		private static readonly List<BaseScript> ms_definedScripts = new List<BaseScript>();
		private static readonly List<Tuple<DateTime, AsyncCallback>> ms_delays = new List<Tuple<DateTime, AsyncCallback>>();
		private static int ms_instanceId;

		public static IScriptHost ScriptHost { get; private set; }

		[SecuritySafeCritical]
		public InternalManager()
		{
			//CultureInfo.DefaultThreadCurrentCulture = CultureInfo.InvariantCulture;
			//CultureInfo.DefaultThreadCurrentUICulture = CultureInfo.InvariantCulture;

			Thread.CurrentThread.CurrentCulture = CultureInfo.InvariantCulture;
			Thread.CurrentThread.CurrentUICulture = CultureInfo.InvariantCulture;

			InitializeAssemblyResolver();
			CitizenTaskScheduler.Create();

			SynchronizationContext.SetSynchronizationContext(new CitizenSynchronizationContext());
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
			ScriptHost = (IScriptHost)Marshal.GetObjectForIUnknown(hostPtr);
			ms_instanceId = instanceId;
		}

		internal static void AddScript(BaseScript script)
		{
			if (!ms_definedScripts.Contains(script))
			{
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

		public void CreateAssembly(byte[] assemblyData, byte[] symbolData)
		{
			CreateAssemblyInternal(assemblyData, symbolData);
		}

		[SecuritySafeCritical]
		private static Assembly CreateAssemblyInternal(byte[] assemblyData, byte[] symbolData)
		{
			var assembly = Assembly.Load(assemblyData, symbolData);
			Debug.WriteLine("Loaded {1} into {0}", AppDomain.CurrentDomain.FriendlyName, assembly.FullName);

			var definedTypes = assembly.GetTypes().Where(t => !t.IsAbstract && t.IsSubclassOf(typeof(BaseScript)));

			foreach (var type in definedTypes)
			{
				try
				{
					var derivedScript = Activator.CreateInstance(type) as BaseScript;

					Debug.WriteLine("Instantiated instance of script {0}.", type.FullName);

					ms_definedScripts.Add(derivedScript);
				}
				catch (Exception e)
				{
					Debug.WriteLine("Failed to instantiate instance of script {0}: {1}", type.FullName, e.ToString());
				}
			}

			//ms_definedScripts.Add(new TestScript());

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

		static Assembly LoadAssembly(string name)
		{
			try
			{
				var assemblyStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".dll")));
				var assemblyBytes = assemblyStream.ReadBytes((int)assemblyStream.BaseStream.Length);

				byte[] symbolBytes = null;

				try
				{
					var symbolStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".dll.mdb")));
					symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
				}
				catch
				{
					try
					{
						var symbolStream = new BinaryReader(new FxStreamWrapper(ScriptHost.OpenHostFile(name + ".pdb")));
						symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
					}
					catch
					{
						// nothing
					}
				}

				return CreateAssemblyInternal(assemblyBytes, symbolBytes);
			}
			catch (Exception e)
			{
				Debug.WriteLine($"Exception loading assembly {name}: {e}");
			}

			return null;
		}

		static Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
		{
			return LoadAssembly(args.Name.Split(',')[0]);
		}

		public static void AddDelay(int delay, AsyncCallback callback)
		{
			ms_delays.Add(Tuple.Create(DateTime.UtcNow.AddMilliseconds(delay), callback));
		}

		public void Tick()
		{
			try
			{
				ScriptContext.GlobalCleanUp();

				var delays = ms_delays.ToArray();
				var now = DateTime.UtcNow;

				foreach (var delay in delays)
				{
					if (now >= delay.Item1)
					{
						delay.Item2(new DummyAsyncResult());

						ms_delays.Remove(delay);
					}
				}

				foreach (var script in ms_definedScripts.ToArray())
				{
					script.ScheduleRun();
				}

				CitizenTaskScheduler.Instance.Tick();
				CitizenSynchronizationContext.Tick();
			}
			catch (Exception e)
			{
				Debug.WriteLine("Error during Tick: {0}", e.ToString());

				throw;
			}
		}

		public void TriggerEvent(string eventName, byte[] argsSerialized, string sourceString)
		{
			try
			{
				var obj = MsgPackDeserializer.Deserialize(argsSerialized, netSource: (sourceString.StartsWith("net") ? sourceString : null)) as List<object> ?? (IEnumerable<object>)new object[0];

				var scripts = ms_definedScripts.ToArray();

				var objArray = obj.ToArray();

				NetworkFunctionManager.HandleEventTrigger(eventName, objArray, sourceString);

				foreach (var script in scripts)
				{
					Task.Factory.StartNew(() => script.EventHandlers.Invoke(eventName, sourceString, objArray)).Unwrap().ContinueWith(a =>
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
				Debug.WriteLine(e.ToString());
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
		public void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retvalSerialized, out int retvalSize)
		{
			var retvalData = FunctionReference.Invoke(refIndex, argsSerialized);

			if (retvalData != null)
			{
				if (m_retvalBuffer == IntPtr.Zero)
				{
					m_retvalBuffer = Marshal.AllocHGlobal(32768);
					m_retvalBufferSize = 32768;
				}

				if (m_retvalBufferSize < retvalData.Length)
				{
					m_retvalBuffer = Marshal.ReAllocHGlobal(m_retvalBuffer, new IntPtr(retvalData.Length));
				}

				Marshal.Copy(retvalData, 0, m_retvalBuffer, retvalData.Length);

				retvalSerialized = m_retvalBuffer;
				retvalSize = retvalData.Length;
			}
			else
			{
				retvalSerialized = IntPtr.Zero;
				retvalSize = 0;
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
	}
}
