using System;

namespace CitizenFX.Core
{
	public delegate object ExternalFunction(params object[] args);

	internal abstract class _ExternalFunction
	{
		internal enum Flags : ulong
		{
			HAS_RETURN_VALUE = (ulong)1 << 32,
			IS_PERSISTENT = (ulong)1 << 33,
			REF_COUNTING = (ulong)1 << 34, // reserved for future usage as an extra option to allow re-invokable remote functions

		}

		internal protected readonly string m_reference;
		internal protected readonly bool m_repeatable;

		internal protected _ExternalFunction(string reference, bool repeatable)
		{
			m_reference = reference;
			m_repeatable = repeatable;
		}

#if REMOTE_FUNCTION_ENABLED
		internal bool IsRemote { get => this is _RemoteFunction; }
#endif
		internal bool IsLocal { get => this is _LocalFunction; }
		internal bool IsRepeatable { get => m_repeatable; }
	}

	public static class DelegateExternals
	{
		public static bool IsInternal(this Delegate callback) => !(callback.Target is _ExternalFunction);
		public static bool IsExternal(this Delegate callback) => callback.Target is _ExternalFunction;
#if REMOTE_FUNCTION_ENABLED
		public static bool IsRemote(this Delegate callback) => callback.Target is _RemoteFunction;
#endif
		public static bool IsLocal(this Delegate callback) => callback.Target is _LocalFunction;
		public static bool IsExternalRepeatable(this Delegate callback) => callback.Target is _ExternalFunction exFunction && exFunction.IsRepeatable;

		public static ExternalFunction AsExternalFunction(this Delegate callback)
		{
			return callback.Target is _ExternalFunction exFunction && exFunction.IsRepeatable
				? (ExternalFunction)Delegate.CreateDelegate(typeof(ExternalFunction), callback.Target, callback.Method)
				: null;
		}

#if REMOTE_FUNCTION_ENABLED
		public static RemoteFunction AsRemoteFunction(this Delegate callback)
		{
			return callback.Target is _RemoteFunction rf && rf.IsRepeatable
				? (RemoteFunction)Delegate.CreateDelegate(typeof(RemoteFunction), callback.Target, callback.Method)
				: null;
		}
#endif

		public static LocalFunction AsLocalFunction(this Delegate callback)
		{
			return callback.Target is _LocalFunction rf && rf.IsRepeatable
				? (LocalFunction)Delegate.CreateDelegate(typeof(LocalFunction), callback.Target, callback.Method)
				: null;
		}

		public static bool AsExternalFunction(this Delegate callback, out ExternalFunction externalFunction) => (externalFunction = AsExternalFunction(callback)) != null;
#if REMOTE_FUNCTION_ENABLED
		public static bool AsRemoteFunction(this Delegate callback, out RemoteFunction remoteFunction) => (remoteFunction = AsRemoteFunction(callback)) != null;
#endif
		public static bool AsLocalFunction(this Delegate callback, out LocalFunction localFunction) => (localFunction = AsLocalFunction(callback)) != null;
	}
}
