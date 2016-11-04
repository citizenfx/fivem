using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.IO;
using System.Linq;
using System.Security;

using System.Reflection;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;
using CitizenFX.Core;

namespace CitizenFX.Core
{
    internal static class RuntimeManager
    {
        public static void Initialize()
        {
            Debug.WriteLine("Hello!");
        }

        public static Guid[] GetImplementedClasses(Guid ifImpl)
        {
            return Assembly.GetExecutingAssembly().GetTypes()
                .Where(a => a.IsClass)
                .Where(a => a.GetInterfaces()
                    .Select(b => b.GetCustomAttribute<GuidAttribute>()?.Value)
                    .Any(b => (b != null && ifImpl == Guid.Parse(b)))
                )
                .Select(a => Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value))
                .ToArray();
        }

        [SecuritySafeCritical]
        public static IntPtr CreateObjectInstance(Guid guid, Guid iid)
        {
            var type = Assembly.GetExecutingAssembly().GetTypes()
                                .Where(a => a.GetCustomAttribute<GuidAttribute>() != null)
                                .First(
                                       a => Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value) == guid);

            return Marshal.GetComInterfaceForObject(
                Activator.CreateInstance(type),
                Assembly.GetExecutingAssembly().GetTypes()
                    .Where(a => a.GetCustomAttribute<GuidAttribute>() != null)
                    .First(a => a.IsInterface && Guid.Parse(a.GetCustomAttribute<GuidAttribute>().Value) == iid));
        }
    }

    public interface IInternalHost
    {
        void InvokeNative(ref fxScriptContext context);
    }

    class InternalHost : MarshalByRefObject, IInternalHost
    {
        private IScriptHost m_host;

        public InternalHost(IScriptHost host)
        {
            m_host = host;
        }

        public void InvokeNative(ref fxScriptContext context)
        {
            m_host.InvokeNative(context);
        }
    }

    interface InternalManagerInterface
    {
        void SetScriptHost(IntPtr host, int instanceId);

        void CreateAssembly(byte[] assemblyData, byte[] symbolData);

        void Tick();

        void TriggerEvent(string eventName, byte[] argsSerialized, string sourceString);

        void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retvalSerialized, out int retvalSize);

        int DuplicateRef(int refIndex);

        void RemoveRef(int refIndex);
    }

    class InternalManager : MarshalByRefObject, InternalManagerInterface
    {
        private static List<BaseScript> ms_definedScripts = new List<BaseScript>();
        private static List<Tuple<DateTime, AsyncCallback>> ms_delays = new List<Tuple<DateTime, AsyncCallback>>();
        private static IScriptHost ms_scriptHost;
        private static int ms_instanceId;

        public static IScriptHost ScriptHost => ms_scriptHost;

        public InternalManager()
        {
            InitializeAssemblyResolver();
            CitizenTaskScheduler.Create();
        }

        [SecuritySafeCritical]
        public void SetScriptHost(IntPtr host, int instanceId)
        {
            ms_scriptHost = (IScriptHost)Marshal.GetObjectForIUnknown(host);
            ms_instanceId = instanceId;
        }

        public void CreateAssembly(byte[] assemblyData, byte[] symbolData)
        {
            CreateAssemblyInternal(assemblyData, symbolData);
        }

        [SecuritySafeCritical]
        private Assembly CreateAssemblyInternal(byte[] assemblyData, byte[] symbolData)
        {
            var assembly = Assembly.Load(assemblyData, symbolData);
            Debug.WriteLine("Loaded {1} into {0}", AppDomain.CurrentDomain.FriendlyName, assembly.FullName);

            var definedTypes = assembly.GetTypes();

            foreach (var type in definedTypes)
            {
                if (type.IsSubclassOf(typeof(BaseScript)))
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
            }

            ms_definedScripts.Add(new TestScript());

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

        Assembly LoadAssembly(string name)
        {
            try
            {
                var assemblyStream = new BinaryReader(new FxStreamWrapper(ms_scriptHost.OpenHostFile(name + ".dll")));
                var assemblyBytes = assemblyStream.ReadBytes((int)assemblyStream.BaseStream.Length);

                byte[] symbolBytes = null;

                try
                {
                    var symbolStream = new BinaryReader(new FxStreamWrapper(ms_scriptHost.OpenHostFile(name + ".dll.mdb")));
                    symbolBytes = symbolStream.ReadBytes((int)symbolStream.BaseStream.Length);
                }
                catch
                {
                    // nothing
                }

                return CreateAssemblyInternal(assemblyBytes, symbolBytes);
            }
            catch { }

            return null;
        }

        Assembly CurrentDomain_AssemblyResolve(object sender, ResolveEventArgs args)
        {
            return LoadAssembly(args.Name.Split(',')[0]);
        }

        public static void AddDelay(int delay, AsyncCallback callback)
        {
            ms_delays.Add(Tuple.Create(DateTime.Now.AddMilliseconds(delay), callback));
        }

        public void Tick()
        {
            try
            {
                var delays = ms_delays.ToArray();
                var now = DateTime.Now;

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
                Debug.WriteLine($"event {eventName} with payload size {argsSerialized.Length}");

                IEnumerable<object> obj = MsgPackDeserializer.Deserialize(argsSerialized) as List<object> ?? (IEnumerable<object>)new object[0];

                var scripts = ms_definedScripts.ToArray();
                var objArray = obj.ToArray();

                foreach (var script in scripts)
                {
                    script.EventHandlers.Invoke(eventName, objArray);
                }
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.ToString());
            }
        }

        [SecuritySafeCritical]
        public static string CanonicalizeRef(int refId)
        {
            IntPtr re = ScriptHost.CanonicalizeRef(refId, ms_instanceId);
            string str = Marshal.PtrToStringAnsi(re);

            GameInterface.fwFree(re);

            return str;
        }

        private IntPtr m_retvalBuffer;
        private int m_retvalBufferSize;

        [SecuritySafeCritical]
        public void CallRef(int refIndex, byte[] argsSerialized, out IntPtr retvalSerialized, out int retvalSize)
        {
            byte[] retvalData = FunctionReference.Invoke(refIndex, argsSerialized);

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
    }

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
            catch (Exception e)
            {
                Debug.WriteLine(e.ToString());

                throw;
            }
        }

        public void Tick()
        {
            m_intManager?.Tick();
        }

        public void TriggerEvent(string eventName, byte[] argsSerialized, int serializedSize, string sourceId)
        {
            try
            {
                m_intManager?.TriggerEvent(eventName, argsSerialized, sourceId);
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
                m_intManager?.CallRef(refIndex, argsSerialized, out retvalSerialized, out retvalSize);
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
                return m_intManager?.DuplicateRef(refIndex) ?? 0;
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
                m_intManager?.RemoveRef(refIndex);
            }
            catch (Exception e)
            {
                Debug.WriteLine(e.ToString());

                throw;
            }
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    [Serializable]
    public struct fxScriptContext
    {
        [MarshalAs(UnmanagedType.ByValArray, SizeConst = 32 * 8)]
        public byte[] functionData;

        public int numArguments;
        public int numResults;

        public ulong nativeIdentifier;
    }

    public class ScriptContext : IDisposable
    {
        private fxScriptContext m_context;
        private List<Action> m_finalizers;

        public ScriptContext()
        {
            Reset();
        }

        public void Reset()
        {
            CleanUp();

            m_context = new fxScriptContext();
            m_context.functionData = new byte[32 * 8];

            m_finalizers = new List<Action>();
        }

        [SecuritySafeCritical]
        public void Push(object arg)
        {
            if (arg.GetType() == typeof(string))
            {
                var str = (string)Convert.ChangeType(arg, typeof(string));
                var b = Encoding.UTF8.GetBytes(str);

                var ptr = Marshal.AllocHGlobal(b.Length + 1);

                Marshal.Copy(b, 0, ptr, b.Length);
                Marshal.WriteByte(ptr, b.Length, 0);
                
                m_finalizers.Add(() => Free(ptr));

                b = BitConverter.GetBytes(ptr.ToInt64());
                Array.Copy(b, 0, m_context.functionData, 8 * m_context.numArguments, 8);
            }
            else if (arg.GetType() == typeof(Vector3))
            {
                var v = (Vector3)arg;

                int start = 8 * m_context.numArguments;
                Array.Clear(m_context.functionData, start, 8 * 3);
                Array.Copy(BitConverter.GetBytes(v.X), 0, m_context.functionData, start, 4);
                Array.Copy(BitConverter.GetBytes(v.Y), 0, m_context.functionData, start + 8, 4);
                Array.Copy(BitConverter.GetBytes(v.Z), 0, m_context.functionData, start + 16, 4);

                m_context.numArguments += 2;
            }
            else if (Marshal.SizeOf(arg.GetType()) <= 8)
            {
                var ptr = Marshal.AllocHGlobal(8);
                try
                {
                    Marshal.WriteInt64(ptr, 0, 0);
                    Marshal.StructureToPtr(arg, ptr, false);

                    Marshal.Copy(ptr, m_context.functionData, 8 * m_context.numArguments, 8);
                }
                finally
                {
                    Marshal.FreeHGlobal(ptr);
                }
            }

            m_context.numArguments++;
        }

        [SecuritySafeCritical]
        private void Free(IntPtr ptr)
        {
            Marshal.FreeHGlobal(ptr);
        }

        [SecuritySafeCritical]
        public T GetResult<T>()
        {
            return (T)GetResult(typeof(T));
        }

        [SecuritySafeCritical]
        public object GetResult(Type type)
        {
            if (type == typeof(string))
            {
                IntPtr nativeUtf8 = new IntPtr(BitConverter.ToInt64(m_context.functionData, 0));

                int len = 0;
                while (Marshal.ReadByte(nativeUtf8, len) != 0)
                {
                    ++len;
                }

                byte[] buffer = new byte[len];
                Marshal.Copy(nativeUtf8, buffer, 0, buffer.Length);
                return Encoding.UTF8.GetString(buffer);
            }
            else if (type == typeof(Vector3))
            {
                float x = BitConverter.ToSingle(m_context.functionData, 0);
                float y = BitConverter.ToSingle(m_context.functionData, 8);
                float z = BitConverter.ToSingle(m_context.functionData, 16);

                return new Vector3(x, y, z);
            }
            else if (Marshal.SizeOf(type) <= 8)
            {
                return GetResultInternal(type);
            }

            return null;
        }

        [SecurityCritical]
        private unsafe object GetResultInternal(Type type)
        {
            fixed (byte* bit = &m_context.functionData[0])
            {
                return Marshal.PtrToStructure(new IntPtr(bit), type);
            }
        }

        [SecuritySafeCritical]
        public void Invoke(ulong nativeIdentifier, IScriptHost scriptHost) => InvokeInternal(nativeIdentifier, scriptHost);

        [SecurityCritical]
        private unsafe void InvokeInternal(ulong nativeIdentifier, IScriptHost scriptHost)
        {
            m_context.nativeIdentifier = nativeIdentifier;

            scriptHost.InvokeNative(ref m_context);
        }


        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (disposing)
            {
                
            }

            CleanUp();
        }

        private void CleanUp()
        {
            m_finalizers?.ForEach(a => a());
        }
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [System.Runtime.InteropServices.Guid("82ec2441-dbb4-4512-81e9-3a98ce9ffcab")]
    [ComImport]
    public interface fxIStream
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        int Read(byte[] data, int size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        int Write(byte[] data, int size);

        [MethodImpl(MethodImplOptions.InternalCall)]
        long Seek(long offset, int origin);

        [MethodImpl(MethodImplOptions.InternalCall)]
        long GetLength();
    }

    public class FxStreamWrapper : Stream
    {
        private fxIStream m_stream;

        public FxStreamWrapper(fxIStream stream)
        {
            m_stream = stream;
        }

        public override void Flush()
        {
            
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            return m_stream.Seek(offset, (int)origin);
        }

        public override void SetLength(long value)
        {
            
        }

        public override int Read(byte[] buffer, int offset, int count)
        {
            byte[] inRead = new byte[count];
            int numRead = m_stream.Read(inRead, count);

            Array.Copy(inRead, 0, buffer, offset, count);

            return numRead;
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            byte[] inWrite = new byte[count];
            Array.Copy(buffer, offset, inWrite, 0, count);

            m_stream.Write(inWrite, count);
        }

        public override bool CanRead => true;

        public override bool CanSeek => true;

        public override bool CanWrite => true;

        public override long Length => m_stream.GetLength();

        public override long Position {
            get { return m_stream.Seek(0, 1); }
            set { m_stream.Seek(value, 0); }
        }
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [System.Runtime.InteropServices.Guid("8ffdc384-4767-4ea2-a935-3bfcad1db7bf")]
    [ComImport]
    public interface IScriptHost
    {
        [MethodImpl(MethodImplOptions.InternalCall)]
        void InvokeNative([MarshalAs(UnmanagedType.Struct)] ref fxScriptContext context);

        [MethodImpl(MethodImplOptions.InternalCall)]
        [return: MarshalAs(UnmanagedType.Interface)]
        fxIStream OpenSystemFile([MarshalAs(UnmanagedType.LPStr)] string fileName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        [return: MarshalAs(UnmanagedType.Interface)]
        fxIStream OpenHostFile([MarshalAs(UnmanagedType.LPStr)] string fileName);

        [MethodImpl(MethodImplOptions.InternalCall)]
        IntPtr CanonicalizeRef(int localRef, int instanceId);
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [System.Runtime.InteropServices.Guid("67b28af1-aaf9-4368-8296-f93afc7bde96")]
    public interface IScriptRuntime
    {
        void Create([MarshalAs(UnmanagedType.Interface)] IScriptHost host);

        void Destroy();

        [PreserveSig]
        IntPtr GetParentObject();

        [PreserveSig]
        void SetParentObject(IntPtr ptr);

        [PreserveSig]
        int GetInstanceId();
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("91b203c7-f95a-4902-b463-722d55098366")]
    public interface IScriptTickRuntime
    {
        void Tick();
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("a2f1b24b-a29f-4121-8162-86901eca8097")]
    public interface IScriptRefRuntime
    {
        void CallRef(int refIndex,
            [In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] argsSerialized,
            int argsSize,
            [Out] out IntPtr retvalSerialized,
            [Out] out int retvalSize);

        int DuplicateRef(int refIndex);

        void RemoveRef(int refIndex);
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [Guid("637140db-24e5-46bf-a8bd-08f2dbac519a")]
    public interface IScriptEventRuntime
    {
        void TriggerEvent([MarshalAs(UnmanagedType.LPStr)] string eventName,
            [In] [MarshalAs(UnmanagedType.LPArray, SizeParamIndex = 2)] byte[] argsSerialized,
            int serializedSize,
            [MarshalAs(UnmanagedType.LPStr)] string sourceId);
    }

    [InterfaceType(ComInterfaceType.InterfaceIsIUnknown)]
    [System.Runtime.InteropServices.Guid("567634c6-3bdd-4d0e-af39-7472aed479b7")]
    public interface IScriptFileHandlingRuntime
    {
        [PreserveSig]
        int HandlesFile([MarshalAs(UnmanagedType.LPStr)] string filename);

        void LoadFile([MarshalAs(UnmanagedType.LPStr)]string scriptFile);
    }
}