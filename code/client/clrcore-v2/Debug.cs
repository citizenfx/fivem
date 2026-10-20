using System;
using System.ComponentModel;
using System.Reflection;
using System.Security;

namespace CitizenFX.Core
{
	[Flags]
	public enum DynFuncExceptions : uint
	{
		/// <summary>
		/// Ignore any exception thrown
		/// /// </summary>
		None = 0x0,

		/// <summary>
		/// If you like to keep the option to ignore calls with incorrect arguments but d
		/// </summary>
		/// <example>
		/// // Disable InvalidCastException logs but still make use of them being ignored
		/// Debug.LogExceptionsOnDynFunc &amp;= ~DynFuncExceptions.InvalidCastException;
		/// </example>
		InvalidCastException = 0x1,

		/// <summary>
		/// Will output parameters vs argument conversion/castability report, helpful for development
		/// </summary>
		/// <remarks>Can have a performance if it's hit often.</remarks>
		ReportTypeMatching = 1u << 31,

		//ReportVerbose = 1u << 30,

		AllExceptions = ~0u >> 2,
		AllReports = ~AllExceptions, // the remaining bits are ours
		
		/// <summary>
		/// Panic mode, give all details
		/// </summary>
		Everything = ~0u
	}

	public static class Debug
	{
		private static string s_debugName = "script:mono";
		public static DynFuncExceptions LogExceptionsOnDynFunc { get; set; } = DynFuncExceptions.Everything;

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
			WriteLine(CreateError(what, where));
		}

		internal static string CreateError(Exception what, string where = null)
		{
			where = where != null ? " in " + where : "";
			return $"^1SCRIPT ERROR{where}: {what.GetType().FullName}: {what.Message}^7\n" + what.StackTrace.ToString();
		}

		/*[SecuritySafeCritical]
		internal static unsafe string FormatStackTrace(byte[] serializedFrames)
		{
			fixed (byte* ptr = serializedFrames)
			{
				return Native.Function.Call<string>((Native.Hash)0xd70c3bca, ptr, serializedFrames.Length);
			}
		}*/

		[EditorBrowsable(EditorBrowsableState.Never)]
		[SecuritySafeCritical]
		public static void WriteException(Exception exception, DynFunc dynFunc, object[] arguments, string methodAnnotation = "dynamic method")
		{
			if (LogExceptionsOnDynFunc == 0)
				return;

			string errorMessage = "";

			if (exception is InvalidCastException && (LogExceptionsOnDynFunc & DynFuncExceptions.InvalidCastException) != 0)
			{
				if ((LogExceptionsOnDynFunc & DynFuncExceptions.ReportTypeMatching) != 0)
				{
					string GetTypeName(Type type)
					{
						return type.Assembly == typeof(Int32).Assembly
							? type.ToString().Substring(type.Namespace.Length + 1)
							: type.ToString();
					}

					MethodInfo methodInfo = dynFunc.GetWrappedMethod();
					ParameterInfo[] parameters = methodInfo.GetParameters();

					string comma = "";
					string argumentsString = "";
					string parametersString = "";

					const string sourceString = " ^2// [Source] parameters are hidden^7";
					string hasSource = "";

					// Test arguments against the parameters
					int a = 0, p = 0;
					for (; p < parameters.Length && a < arguments.Length; ++p)
					{
						ParameterInfo pInfo = parameters[p];
						Type pType = pInfo.ParameterType;
						string pName = GetTypeName(pType);

						if (pInfo.GetCustomAttribute<SourceAttribute>() != null)
						{
							hasSource = sourceString;
						}
						else
						{
							Type aType = arguments[a]?.GetType();
							string aName = aType is null ? "null" : GetTypeName(aType);

							bool assignable = pType.IsAssignableFrom(aType)
								|| (typeof(IConvertible).IsAssignableFrom(pType) && typeof(IConvertible).IsAssignableFrom(aType));

							int maxNameSize = Math.Max(aName.Length, pName.Length);
							argumentsString += comma + (assignable ? aName.PadRight(maxNameSize) : $"^1{aName}^7".PadRight(maxNameSize + 4));
							parametersString += comma + (assignable ? pName.PadRight(maxNameSize) : $"^1{pName}^7".PadRight(maxNameSize + 4));

							++a;
						}

						comma = ", ";
					}

					// add missing parameters
					for (; p < parameters.Length; ++p)
					{
						ParameterInfo pInfo = parameters[p];
						if (pInfo.GetCustomAttribute<SourceAttribute>() != null)
						{
							hasSource = sourceString;
						}

						parametersString += comma + $"^1{GetTypeName(pInfo.ParameterType)}^7";
						comma = ", ";
					}

					// add excess arguments
					for (; a < arguments.Length; ++a)
					{
						Type aType = arguments[a]?.GetType();
						string aName = aType is null ? "null" : GetTypeName(aType);
						argumentsString += comma + aName;
						comma = ", ";
					}

					errorMessage += $"\n  expected: ({parametersString}){hasSource}\n  received: ({argumentsString})" +
						$"\n  * Any type in ^1red^7 is not convertible, castable, or simply missing and requires attention.";
				}
			}

			// Write the actual error/exception
			WriteLine($"^1SCRIPT ERROR: Could not invoke {methodAnnotation} {dynFunc.Method.Name}^7" +
				errorMessage +
				$"\n^3Failed with exception:\n{exception}^7");
		}
	}
}
