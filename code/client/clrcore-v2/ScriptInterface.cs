using System;
using System.Diagnostics.CodeAnalysis;
using System.Runtime.CompilerServices;
using System.Security;
using System.Threading;

#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;
#else
using ContextType = CitizenFX.Core.RageScriptContext;
#endif

/*
* Notes while working on this environment:
*/

namespace CitizenFX.Core
{
	internal static class ScriptInterface
	{
		private static UIntPtr s_runtime;
		internal static int InstanceId { get; private set; }
		internal static string ResourceName { get; private set; }
		internal static CString CResourceName { get; private set; }

		private static unsafe ScriptSharedData* s_sharedData;

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
		private static extern unsafe byte[] InvokeFunctionReference(UIntPtr host, string refId, byte[] argsSerialized);

		[SecurityCritical]
		internal static unsafe byte[] InvokeFunctionReference(string refId, byte[] argsSerialized) => InvokeFunctionReference(s_runtime, refId, argsSerialized);

		[SecurityCritical, MethodImpl(MethodImplOptions.InternalCall)]
		private static extern unsafe bool ReadAssembly(UIntPtr host, string file, out byte[] assembly, out byte[] symbols);

		[SecurityCritical]
		internal static unsafe bool ReadAssembly(string file, out byte[] assembly, out byte[] symbols) => ReadAssembly(s_runtime, file, out assembly, out symbols);

		/// <summary>
		/// Schedule a call-in at the given time, uses CAS to make sure we only overwrite if the given time is earlier than the stored one.
		/// </summary>
		/// <param name="time">Next time to request a call in</param>
		[SecuritySafeCritical]
		internal static unsafe void RequestTick(ulong time)
		{			
			ulong prevTime = (ulong)Interlocked.Read(ref s_sharedData->m_scheduledTimeAsLong);
			while (time < prevTime)
			{
				prevTime = (ulong)Interlocked.CompareExchange(ref s_sharedData->m_scheduledTimeAsLong, (long)time, (long)prevTime);
			}
		}

		/// <summary>
		/// Schedule a call-in for the next frame.
		/// </summary>
		[SecuritySafeCritical]
		public static unsafe void RequestTickNextFrame() => s_sharedData->m_scheduledTime = 0UL; // 64 bit read/writes – while aligned – are atomic on 64 bit machines

		#endregion

		#region Called by Native
		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		private static unsafe void Initialize(string resourceName, UIntPtr runtime, int instanceId, ScriptSharedData* sharedData)
		{
			s_runtime = runtime;
			InstanceId = instanceId;
			ResourceName = resourceName;
			CResourceName = resourceName;
			s_sharedData = sharedData;

			Resource.Current = new Resource(resourceName);
			Debug.Initialize(resourceName);
			Scheduler.Initialize();

			ExportsManager.Initialize(resourceName);
#if REMOTE_FUNCTION_ENABLED
			ExternalsManager.Initialize(resourceName, instanceId);
#endif
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static void Tick(ulong hostTime, bool profiling)
		{
			Scheduler.CurrentTime = (TimePoint)hostTime;
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
		internal static unsafe void TriggerEvent(string eventName, byte* argsSerialized, int serializedSize, string sourceString, ulong hostTime, bool profiling)
		{
			Scheduler.CurrentTime = (TimePoint)hostTime;
			Profiler.IsProfiling = profiling;
			
			Binding origin = !sourceString.StartsWith("net") ? Binding.Local : Binding.Remote;

#if IS_FXSERVER
			sourceString = (origin == Binding.Remote || sourceString.StartsWith("internal-net")) ? sourceString : null;
#else
			sourceString = origin == Binding.Remote ? sourceString : null;
#endif

			object[] args = null; // will make sure we only deserialize it once
#if REMOTE_FUNCTION_ENABLED
			if (!ExternalsManager.IncomingRequest(eventName, sourceString, origin, argsSerialized, serializedSize, ref args))
#endif
			{
				if (!ExportsManager.IncomingRequest(eventName, sourceString, origin, argsSerialized, serializedSize, ref args))
				{
					// if a remote function or export has consumed this event then it surely wasn't meant for event handlers
					EventsManager.IncomingEvent(eventName, sourceString, origin, argsSerialized, serializedSize, args);
				}
			}
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static unsafe void LoadAssembly(string name, ulong hostTime, bool profiling)
		{
			Scheduler.CurrentTime = (TimePoint)hostTime;
			Profiler.IsProfiling = profiling;

			ScriptManager.LoadAssembly(name, true);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static unsafe ulong CallRef(int refIndex, byte* argsSerialized, uint argsSize, out byte[] retval, ulong hostTime, bool profiling)
		{
			Scheduler.CurrentTime = (TimePoint)hostTime;
			Profiler.IsProfiling = profiling;

			ReferenceFunctionManager.IncomingCall(refIndex, argsSerialized, argsSize, out retval);

			return Scheduler.NextTaskTime();
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static int DuplicateRef(int refIndex)
		{
			return ReferenceFunctionManager.IncrementReference(refIndex);
		}

		[SecurityCritical, SuppressMessage("System.Diagnostics.CodeAnalysis", "IDE0051", Justification = "Called by host")]
		internal static void RemoveRef(int refIndex)
		{
			ReferenceFunctionManager.DecrementReference(refIndex);
		}

#endregion
	}
}
