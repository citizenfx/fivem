using System;
using System.IO;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	[Guid("C068E0AB-DD9C-48F2-A7F3-69E866D27F17")]
	class MonoScriptRuntime : IScriptRuntime, IScriptFileHandlingRuntime, IScriptTickRuntime, IScriptEventRuntime, IScriptRefRuntime
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

				m_intManager = (InternalManager)m_appDomain.CreateInstanceAndUnwrap(typeof(InternalManager).Assembly.FullName, typeof(InternalManager).FullName);
				m_intManager.SetScriptHost(Marshal.GetIUnknownForObject(host), m_instanceId);
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

		public PushRuntime GetPushRuntime()
		{
			return new PushRuntime(this);
		}
	}
}