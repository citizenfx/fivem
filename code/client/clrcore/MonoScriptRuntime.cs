using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	[Guid("C068E0AB-DD9C-48F2-A7F3-69E866D27F17")]
	class MonoScriptRuntime : IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime, IScriptMemInfoRuntime
	{
		private IScriptHost m_scriptHost;
		private readonly int m_instanceId;
		private AppDomain m_appDomain;
		private InternalManager m_intManager;
		private IntPtr m_parentObject;

		private static readonly Random ms_random = new Random();

		public MonoScriptRuntime()
		{
			m_instanceId = ms_random.Next();
		}

		[SecuritySafeCritical]
		public void Create(IScriptHost host)
		{
			try
			{
				m_scriptHost = host;

				m_appDomain = AppDomain.CreateDomain($"ScriptDomain_{m_instanceId}");
				m_appDomain.SetupInformation.ConfigurationFile = "dummy.config";

				m_intManager = (InternalManager)m_appDomain.CreateInstanceAndUnwrap(typeof(InternalManager).Assembly.FullName, typeof(InternalManager).FullName);

				// TODO: figure out a cleaner solution to Mono JIT corruption so server doesn't have to be slower
#if IS_FXSERVER
				m_intManager.SetScriptHost(new WrapScriptHost(host), m_instanceId);
#else
				m_intManager.SetScriptHost(Marshal.GetIUnknownForObject(host), m_instanceId);
#endif
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				if (e.InnerException != null)
				{
					Debug.WriteLine(e.InnerException.ToString());
				}

				throw;
			}
		}

		public void Destroy()
		{
			AppDomain.Unload(m_appDomain);

			m_appDomain = null;
			m_intManager = null;
		}

		public IntPtr GetParentObject()
		{
			return m_parentObject;
		}

		public void SetParentObject(IntPtr ptr)
		{
			m_parentObject = ptr;
		}

		public int GetInstanceId()
		{
			return m_instanceId;
		}

		public int HandlesFile(string filename)
		{
			return (filename.EndsWith(".net.dll") ? 1 : 0);
		}

		[SecuritySafeCritical]
		public void LoadFile(string scriptFile)
		{
			try
			{
				using (GetPushRuntime())
				{
					var assemblyStream = new BinaryReader(new FxStreamWrapper(m_scriptHost.OpenHostFile(scriptFile)));
					var assemblyBytes = assemblyStream.ReadBytes((int)assemblyStream.BaseStream.Length);

					byte[] symbolBytes = null;

					try
					{
						var symbolStream = new BinaryReader(new FxStreamWrapper(m_scriptHost.OpenHostFile(scriptFile + ".mdb")));
						symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
					}
					catch
					{
						// nothing
					}

					m_intManager.CreateAssembly(assemblyBytes, symbolBytes);
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				throw;
			}
		}

		public void Tick()
		{
			using (GetPushRuntime())
			{
				m_intManager?.Tick();
			}
		}

		public void TriggerEvent(string eventName, byte[] argsSerialized, int serializedSize, string sourceId)
		{
			try
			{
				using (GetPushRuntime())
				{
					m_intManager?.TriggerEvent(eventName, argsSerialized, sourceId);
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				throw;
			}
		}

		public void CallRef(int refIndex, byte[] argsSerialized, int argsSize, out IntPtr retvalSerialized, out int retvalSize)
		{
			retvalSerialized = IntPtr.Zero;
			retvalSize = 0;

			try
			{
				using (GetPushRuntime())
				{
					m_intManager?.CallRef(refIndex, argsSerialized, out retvalSerialized, out retvalSize);
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				throw;
			}
		}

		public int DuplicateRef(int refIndex)
		{
			try
			{
				using (GetPushRuntime())
				{
					return m_intManager?.DuplicateRef(refIndex) ?? 0;
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				throw;
			}
		}

		public void RemoveRef(int refIndex)
		{
			try
			{
				using (GetPushRuntime())
				{
					m_intManager?.RemoveRef(refIndex);
				}
			}
			catch (Exception e)
			{
				Debug.WriteLine(e.ToString());

				throw;
			}
		}

		public void RequestMemoryUsage()
		{
		}

		public void GetMemoryUsage([Out] out ulong memoryUsage)
		{
			using (GetPushRuntime())
			{
				memoryUsage = m_intManager?.GetMemoryUsage() ?? 0;
			}
		}

		public PushRuntime GetPushRuntime()
		{
			return new PushRuntime(this);
		}

		public class WrapIStream : MarshalByRefObject, fxIStream
		{
			private readonly fxIStream m_realStream;

			public WrapIStream(fxIStream realStream)
			{
				m_realStream = realStream;
			}

			public int Read([Out] byte[] data, int size)
			{
				return m_realStream.Read(data, size);
			}

			public int Write(byte[] data, int size)
			{
				return m_realStream.Write(data, size);
			}

			public long Seek(long offset, int origin)
			{
				return m_realStream.Seek(offset, origin);
			}

			public long GetLength()
			{
				return m_realStream.GetLength();
			}
		}

		public class WrapScriptHost : MarshalByRefObject, IScriptHost
		{
			private readonly IScriptHost m_realHost;

			public WrapScriptHost(IScriptHost realHost)
			{
				m_realHost = realHost;
			}

			public void InvokeNative([MarshalAs(UnmanagedType.Struct)] IntPtr context)
			{
				m_realHost.InvokeNative(context);
			}

			[return: MarshalAs(UnmanagedType.Interface)]
			public fxIStream OpenSystemFile([MarshalAs(UnmanagedType.LPStr)] string fileName)
			{
				return new WrapIStream(m_realHost.OpenSystemFile(fileName));
			}

			[return: MarshalAs(UnmanagedType.Interface)]
			public fxIStream OpenHostFile([MarshalAs(UnmanagedType.LPStr)] string fileName)
			{
				return new WrapIStream(m_realHost.OpenHostFile(fileName));
			}

			public IntPtr CanonicalizeRef(int localRef, int instanceId)
			{
				return m_realHost.CanonicalizeRef(localRef, instanceId);
			}

			public void ScriptTrace([MarshalAs(UnmanagedType.LPStr)] string message)
			{
				m_realHost.ScriptTrace(message);
			}

			[SecurityCritical]
			public override object InitializeLifetimeService()
			{
				return null;
			}
		}
	}
}
