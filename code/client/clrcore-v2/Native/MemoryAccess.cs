using System;
using System.Runtime.CompilerServices;
using System.Security;

namespace CitizenFX.Core.Native
{
	[SecurityCritical]
	internal static class MemoryAccess
	{
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe IntPtr Deref(IntPtr ptr, int offset)
		{
			return *(IntPtr*)(ptr + offset);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe IntPtr DerefIfNotNull(IntPtr ptr, int offset)
		{
			return ptr != IntPtr.Zero
				? *(IntPtr*)(ptr + offset)
				: IntPtr.Zero;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe IntPtr Deref(IntPtr ptr, int offset, int postDerefOffset)
		{
			return *(IntPtr*)(ptr + offset) + postDerefOffset;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe IntPtr DerefIfNotNull(IntPtr ptr, int offset, int postDerefOffset)
		{
			return ptr != IntPtr.Zero
				? *(IntPtr*)(ptr + offset) + postDerefOffset
				: IntPtr.Zero;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe T Read<T>(IntPtr ptr, int offset) where T : unmanaged
		{
			return *(T*)(ptr + offset);
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe T ReadIfNotNull<T>(IntPtr ptr, int offset, T orDefault) where T : unmanaged
		{
			return ptr != IntPtr.Zero
				? *(T*)(ptr + offset)
				: orDefault;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe T ReadIfNotNull<T>(IntPtr ptr, int offset, ref T setOrDefault) where T : unmanaged
		{
			return ptr != IntPtr.Zero
				? setOrDefault = *(T*)(ptr + offset)
				: setOrDefault;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void Write<T>(IntPtr ptr, int offset, T value) where T : unmanaged
		{
			*(T*)(ptr + offset) = value;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void WriteIfNotNull<T>(IntPtr ptr, int offset, T value) where T : unmanaged
		{
			if (ptr != IntPtr.Zero)
			{
				*(T*)(ptr + offset) = value;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe bool IsBitSetIfNotNull(IntPtr ptr, int offset, int bit, bool orDefault)
		{
			return ptr != IntPtr.Zero
				? (*(int*)(ptr + offset) & (1 << bit)) != 0
				: orDefault;
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void WriteBitIfNotNull(IntPtr ptr, int offset, int bit, bool value)
		{
			if (ptr != IntPtr.Zero)
			{
				if (value)
				{
					*(int*)(ptr + offset) |= 1 << bit;
				}
				else
				{
					*(int*)(ptr + offset) &= ~(1 << bit);
				}
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void SetBitIfNotNull(IntPtr ptr, int offset, int bit)
		{
			if (ptr != IntPtr.Zero)
			{
				*(int*)(ptr + offset) |= 1 << bit;
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void ClearBitIfNotNull(IntPtr ptr, int offset, int bit)
		{
			if (ptr != IntPtr.Zero)
			{
				*(int*)(ptr + offset) &= ~(1 << bit);
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe bool IsBitSet(IntPtr pointer, int offset, int bit) => (*(int*)(pointer + offset) & (1 << bit)) != 0;

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void ClearBit(IntPtr pointer, int offset, int bit) => *(int*)(pointer + offset) &= ~(1 << bit);

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe void SetBit(IntPtr pointer, int offset, int bit) => *(int*)(pointer + offset) |= 1 << bit;

		/// <remarks>
		/// Purely used to circumvent <see cref="IntPtr"/> operator+'s <see langword="internal"/> accessibility in game libraries
		/// TODO: make mscorlib IntPtr operator+ public and [SecurityCritical]
		/// </remarks>
		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		public static unsafe IntPtr Offset(IntPtr ptr, int offset)
		{
			return ptr + offset;
		}

		// Backward compatibility
		public static float ReadWorldGravity() => throw new NotImplementedException();
		public static void WriteWorldGravity(float value) => throw new NotImplementedException();
	}
}
