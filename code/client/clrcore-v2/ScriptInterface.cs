using System;
using System.Diagnostics.CodeAnalysis;
using System.Reflection;
using System.Runtime.CompilerServices;
using System.Security;

#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;
#else
using ContextType = CitizenFX.Core.RageScriptContext;
#endif

namespace CitizenFX.Core
{
	internal static class ScriptInterface
	{
		private static UIntPtr s_runtime;
		internal static int InstanceId { get; private set; }
		internal static string ResourceName { get; private set; }
		internal static CString CResourceName { get; private set; }

		#region Callable from C#

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void CFree(IntPtr ptr);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void Print(string channel, string text);

		/*[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern ulong GetMemoryUsage();

		[SecurityCritical]
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool SnapshotStackBoundary(out byte[] data);

		[SecurityCritical]
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool WalkStackBoundary(string resourceName, byte[] start, byte[] end, out byte[] blob);*/

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern bool ProfilerIsRecording();

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void ProfilerEnterScope(string name);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern void ProfilerExitScope();

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern UIntPtr GetNative(ulong hash);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		internal static extern unsafe bool InvokeNative(UIntPtr native, ref ContextType context, ulong hash);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		private static extern unsafe byte[] CanonicalizeRef(UIntPtr host, int refId);

		[SecurityCritical]
		internal static unsafe byte[] CanonicalizeRef(int refId) => CanonicalizeRef(s_runtime, refId);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		private static extern unsafe bool ReadAssembly(UIntPtr host, string file, out byte[] assembly, out byte[] symbols);

		[SecurityCritical]
		internal static unsafe bool ReadAssembly(string file, out byte[] assembly, out byte[] symbols) => ReadAssembly(s_runtime, file, out assembly, out symbols);

		#endregion

		#region Called by Native
		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static void Initialize(string resourceName, UIntPtr runtime, int instanceId)
		{
			s_runtime = runtime;
			InstanceId = instanceId;
			ResourceName = resourceName;
			CResourceName = resourceName;

			Resource.Current = new Resource(resourceName);
			Debug.Initialize(resourceName);
			ExportsManager.Initialize(resourceName);
#if REMOTE_FUNCTION_ENABLED
			ExternalsManager.Initialize(resourceName, instanceId);
#endif
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static void Tick(bool profiling)
		{
			Profiler.IsProfiling = profiling;

			try
			{
				ScriptContext.CleanUp();
				Scheduler.Update();
			}
			catch (Exception e)
			{
				Debug.PrintError(e, "Tick()");
			}
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static unsafe void TriggerEvent(string eventName, byte* argsSerialized, int serializedSize, string sourceString)
		{
#if IS_FXSERVER
			var netSource = (sourceString.StartsWith("net") || sourceString.StartsWith("internal-net")) ? sourceString : null;
#else
			var netSource = sourceString.StartsWith("net") ? sourceString : null;
#endif
			Binding origin = string.IsNullOrEmpty(sourceString) ? Binding.LOCAL : Binding.REMOTE;

			object[] args = null; // will make sure we only deserialize it once
#if REMOTE_FUNCTION_ENABLED
			if (ExternalsManager.IncomingRequest(eventName, netSource, origin, argsSerialized, serializedSize, ref args))
				return;
#endif

			if (ExportsManager.IncomingRequest(eventName, netSource, origin, argsSerialized, serializedSize, ref args))
				return;

			// if a remote function or export has consumed this event then it surely wasn't meant for event handlers
			EventManager.IncomingEvent(eventName, netSource, origin, argsSerialized, serializedSize, args);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static unsafe void LoadAssembly(string name)
		{
			ScriptManager.LoadAssembly(name, true);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static unsafe void CallRef(int refIndex, byte* argsSerialized, uint argsSize, out IntPtr retvalSerialized, out uint retvalSize)
		{
			ReferenceFunctionManager.IncomingCall(refIndex, argsSerialized, argsSize, out retvalSerialized, out retvalSize);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static int DuplicateRef(int refIndex)
		{
			return ReferenceFunctionManager.Duplicate(refIndex);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static void RemoveRef(int refIndex)
		{
			ReferenceFunctionManager.Remove(refIndex);
		}

#endregion
	}
}
