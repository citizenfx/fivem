using System;
using System.Security;

namespace CitizenFX.Core
{
	public static class Profiler
	{
		public static bool IsProfiling { get; internal set; }

		[SecuritySafeCritical]
		internal static void EnterScope(string scopeName)
		{
			if (IsProfiling)
				ScriptInterface.ProfilerEnterScope(scopeName);
		}

		/// <param name="scopeName">Will only be evaluated when we're profiling, good for formatting strings</param>
		[SecuritySafeCritical]
		internal static void EnterScope(Func<string> scopeName)
		{
			if (IsProfiling)
				ScriptInterface.ProfilerEnterScope(scopeName());
		}

		[SecuritySafeCritical]
		internal static void ExitScope()
		{
			if (IsProfiling)
				ScriptInterface.ProfilerExitScope();
		}
	}

	/// <remarks>Mind you: using this over Profiler.EnterScope()/Profiler.ExitScope() has a slightly higher performance impact.</remarks>
	public struct ProfilerScope : IDisposable
	{
		/// <remarks>Mind you: using this over Profiler.EnterScope()/Profiler.ExitScope() has a slightly higher performance impact.</remarks>
		public ProfilerScope(string name = "C# scope")
		{
			Profiler.EnterScope(name);
		}

		/// <param name="name">Will only be evaluated when we're profiling, good for formatting strings</param>
		/// <remarks>Mind you: using this over Profiler.EnterScope()/Profiler.ExitScope() has a slightly higher performance impact.</remarks>
		public ProfilerScope(Func<string> name)
		{
			Profiler.EnterScope(name);
		}

		public void Dispose()
		{
			Profiler.ExitScope();
		}
	}
}
