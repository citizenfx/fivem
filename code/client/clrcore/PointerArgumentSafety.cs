using System;
using System.Collections.Concurrent;
using System.Collections.Generic;

namespace CitizenFX.Core.Native
{
	internal static partial class PointerArgumentSafety
	{
		internal delegate bool CleanerDelegate(Type type);

		private static Dictionary<ulong, CleanerDelegate> ms_cleaners = new Dictionary<ulong, CleanerDelegate>();
		private static Dictionary<ulong, ulong> ms_pointerArgumentMasks = new Dictionary<ulong, ulong>();
		private static Dictionary<ulong, ulong> ms_stringArgumentMasks = new Dictionary<ulong, ulong>();
		private static ConcurrentDictionary<ulong, ulong> ms_nativeMapCache = new ConcurrentDictionary<ulong, ulong>();

		private static void AddResultCleaner(ulong hash, CleanerDelegate cleaner)
		{
			ms_cleaners[MapNative(hash)] = cleaner;
		}

		private static void AddPointerArgumentMasks(ulong hash, ulong pointerMask, ulong stringMask)
		{
			hash = MapNative(hash);
			ms_pointerArgumentMasks[hash] = pointerMask;
			ms_stringArgumentMasks[hash] = stringMask;
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

		internal static void CheckArguments(ulong hash, InputArgument[] arguments)
		{
#if GTA_FIVE
			if (!ms_pointerArgumentMasks.TryGetValue(MapNative(hash), out var pointerMask))
			{
				return;
			}

			ms_stringArgumentMasks.TryGetValue(MapNative(hash), out var stringMask);

			for (var index = 0; index < arguments.Length && index < 64; ++index)
			{
				var bit = 1UL << index;
				if ((pointerMask & bit) == 0)
				{
					continue;
				}

				var expectString = (stringMask & bit) != 0;
				var value = arguments[index]?.Value;
				if (IsNullPointer(value) || value is OutputArgument || (expectString && value is string))
				{
					continue;
				}

				throw new ArgumentException($"native {hash:X16}: arg[{index}] expected a managed pointer " + (expectString ? "or string " : ""));
			}
#endif
		}

		private static bool IsNullPointer(object value)
		{
			if (value == null || value is IntPtr pointer && pointer == IntPtr.Zero)
			{
				return true;
			}

			if (value.GetType().IsEnum)
			{
				return Convert.ToUInt64(value) == 0;
			}

			switch (value)
			{
				case sbyte number: return number == 0;
				case byte number: return number == 0;
				case short number: return number == 0;
				case ushort number: return number == 0;
				case int number: return number == 0;
				case uint number: return number == 0;
				case long number: return number == 0;
				case ulong number: return number == 0;
				case float number: return number == 0;
				case double number: return number == 0;
				default: return false;
			}
		}
	}
}
