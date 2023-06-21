using System;
using System.Security;
using System.Runtime.InteropServices;
using System.Text;

namespace CitizenFX.Core
{
	[SecuritySafeCritical]
	public readonly ref struct OutString
	{
#pragma warning disable 0649 // this type is reinterpreted i.e.: (OutString*)ptr
		private unsafe readonly byte* data;
#pragma warning restore 0649

		/// <summary>
		/// A managed string that holds a conerted copy of the unmanaged ANSI string. If ptr is null, the method returns a null string.
		/// </summary>
		/// <param name="str"></param>
		[SecuritySafeCritical]
		public static unsafe implicit operator string(in OutString str) => Marshal.PtrToStringAnsi((IntPtr)str.data);

		/// <summary>
		/// A managed byte[] that holds a copy of the unmanaged ANSI string. If ptr is null, the method returns a null string.
		/// </summary>
		/// <param name="str"></param>
		[SecuritySafeCritical]
		public static unsafe explicit operator byte[](in OutString str)
		{
			// TODO: PERF: check if moving this to an internal C++ backed function will be faster
			byte* end = str.data;
			if (end != null)
			{
				for (; *end != 0x0; ++end) ; // find the end, excluding the '\0'
				long length = end - str.data;

				byte[] array = new byte[length];
				Marshal.Copy((IntPtr)str.data, array, 0, (int)length);
				return array;
			}

			return null;
		}

		public override string ToString() => (string)this;

		/// <summary>
		/// Creates a null terminated C-string out of the returned string
		/// </summary>
		/// <returns>null terminated C-string</returns>
		[SecuritySafeCritical]
		public unsafe CString ToCString() => CString.Create(data);

		/// <summary>
		/// Retrieves a substring from this string. Starting at the specified <paramref name="startIndex"/> in strides of <see cref="byte"/>
		/// </summary>
		/// <param name="startIndex">Character index to start the new string with</param>
		/// <param name="length">Amount of characters requested, after <paramref name="startIndex"/></param>
		/// <returns><see cref="string"/> starting from <paramref name="startIndex"/>, <see cref="string.Empty"/> if we passed the end, or <see langword="null"/></returns>
		[SecuritySafeCritical]
		public unsafe string SubString(uint startIndex, uint length = uint.MaxValue)
		{
			byte* end = data;
			if (end == null)
			{
				return null;
			}

			for (; *end != 0x0; ++end); // find the end, i.e.: '\0'
			long maxLength = end - data;

			if (startIndex == maxLength)
			{
				return string.Empty;
			}
			else if (startIndex > maxLength)
			{
				throw new ArgumentOutOfRangeException(nameof(startIndex));
			}

			long copyLength = Math.Min(maxLength - startIndex, length);
			return Encoding.UTF8.GetString(data + startIndex, (int)copyLength);
		}

		/// <summary>
		/// Retrieves a substring from this string. Starting at the specified <paramref name="startIndex"/> in strides of <see cref="byte"/>
		/// </summary>
		/// <param name="startIndex">Character index to start the new string with</param>
		/// <param name="length">Amount of characters requested, after <paramref name="startIndex"/></param>
		/// <returns><see cref="CString"/> starting from <paramref name="startIndex"/>, <see cref="CString.Empty"/> if we passed the end, or <see langword="null"/></returns>
		[SecuritySafeCritical]
		public unsafe CString SubCString(uint startIndex, uint length = uint.MaxValue)
		{
			byte* end = data;
			if (end == null)
			{
				return null;
			}

			for (; *end != 0x0; ++end) ; // find the end, i.e.: '\0'
			long maxLength = end - data;

			if (startIndex == maxLength)
			{
				return CString.Empty;
			}
			else if (startIndex > maxLength)
			{
				throw new ArgumentOutOfRangeException(nameof(startIndex));
			}

			long copyLength = Math.Min(maxLength - startIndex, length);
			return CString.Create(data + startIndex, (uint)copyLength);
		}
	}
}
