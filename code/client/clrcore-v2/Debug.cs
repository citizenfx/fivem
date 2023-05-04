using System;
using System.Security;

namespace CitizenFX.Core
{
	public static class Debug
	{
		private static string s_debugName = "script:mono";
		public static bool LogInvalidCastExceptionsOnDynFunc { get; set; } = true;

		internal static void Initialize(string resourceName)
		{
			s_debugName = string.IsNullOrWhiteSpace(resourceName) ? "script:mono" : "script:" + resourceName;
		}

		[SecuritySafeCritical]
		public static void Write(string data) => ScriptInterface.Print(s_debugName, data);
		public static void Write(object obj) => Write(obj.ToString());
		public static void Write(string format, params object[] args) => Write(string.Format(format, args));

		public static void WriteLine() => Write("\n");
		public static void WriteLine(string data) => Write(data + "\n");
		public static void WriteLine(object obj) => Write(obj.ToString() + "\n");
		public static void WriteLine(string format, params object[] args) => Write(string.Format(format, args) + "\n");

		internal static void PrintError(Exception what, string where = null)
		{
			where = where != null ? " in " + where : "";
			WriteLine($"^1SCRIPT ERROR{where}: {what.GetType().FullName}: {what.Message}^7");
			WriteLine(what.StackTrace.ToString());
		}

		/*[SecuritySafeCritical]
		internal static unsafe string FormatStackTrace(byte[] serializedFrames)
		{
			fixed (byte* ptr = serializedFrames)
			{
				return Native.Function.Call<string>((Native.Hash)0xd70c3bca, ptr, serializedFrames.Length);
			}
		}*/

		/// <summary>
		/// Simple logic to determine if the InvalidCastException came from the DynFunc and if so if we should print it or ignore it
		/// </summary>
		/// <param name="exc">the thrown exception in question</param>
		/// <param name="func">the DynFunc that was being executed</param>
		/// <returns>true if we should still print/log it</returns>
		[SecuritySafeCritical]
		internal static bool ShouldWeLogDynFuncError(Exception exc, DynFunc func)
		{
			return exc.TargetSite != func.Method
				|| (!(exc is InvalidCastException) || LogInvalidCastExceptionsOnDynFunc);
		}
	}
}
