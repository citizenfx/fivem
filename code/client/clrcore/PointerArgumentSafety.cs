using System;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace CitizenFX.Core.Native
{
	internal static partial class PointerArgumentSafety
	{
		internal delegate bool CleanerDelegate(Type type);

		private static Dictionary<ulong, CleanerDelegate> ms_cleaners = new Dictionary<ulong, CleanerDelegate>();
		private static ConcurrentDictionary<ulong, ulong> ms_nativeMapCache = new ConcurrentDictionary<ulong, ulong>();

		private static void AddResultCleaner(ulong hash, CleanerDelegate cleaner)
		{
			ms_cleaners[MapNative(hash)] = cleaner;
		}

		private static bool IsNonScalar(Type type)
		{
			return type == typeof(string) || type == typeof(object);
		}

		private static bool ResultCleaner_float(Type type)
		{
			return IsNonScalar(type);
		}

		private static bool ResultCleaner_int(Type type)
		{
			return IsNonScalar(type);
		}

		private static bool ResultCleaner_bool(Type type)
		{
			return IsNonScalar(type);
		}

		private static bool ResultCleaner_string(Type type)
		{
			return type != typeof(string);
		}

		private static bool ResultCleaner_FuncRef(Type type)
		{
			return type != typeof(string) && type != typeof(IntPtr);
		}

		private static ulong MapNative(ulong hash)
		{
#if GTA_FIVE
			if (ms_nativeMapCache.TryGetValue(hash, out var newHash))
			{
				return newHash;
			}

			newHash = ScriptContext.MapNativeWrap(hash);
			ms_nativeMapCache[hash] = newHash;
			hash = newHash;
#endif

			return hash;
		}

		internal static bool ShouldClean(ulong hash, Type type)
		{
#if GTA_FIVE
			if (ms_cleaners.TryGetValue(MapNative(hash), out var cleaner))
			{
				return cleaner(type);
			}
#endif

			return false;
		}
	}
}
