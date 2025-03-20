using System;
using System.Linq;
using System.Security;
using System.Runtime.InteropServices;
using System.Text;

namespace CitizenFX.Core
{
	/// <summary>
	/// A null-terminated byte string. Dedicated to interop with native code, equivalent to a (non-wide) c-string.
	/// </summary>
	/// <remarks>In general don't use this type where you would normally use <see cref="string"/> in your .NET code, it needs to do conversion between UTF16 and UTF8 and is therefore slower.
	/// This type and its conversions are more performant in regards to interop with native code (C++), and other runtimes, for these cases you may want to use this.</remarks>
	[SecuritySafeCritical]
	public sealed class CString : IComparable, IComparable<CString>, ICloneable, IEquatable<CString>/*, IConvertible, IEnumerable, IEnumerable<byte>*/
	{
		// Notes while working on this type:
		// 1. Constructors and all other methods should never leave member variable `value` as null, if this type exists then its contents must also exist,
		// 2. Behavior and expectations of this type should be equivalent to the `string` type.
		//
		// Performance notes:
		// 1. This type showed a speed increase of ~43% to ~47% when using strings while interfacing with C++ code,
		// 2. Moving most methods to C++ and using InternalCall proved to be 1/3 slower, unless we build it into the runtime directly like `string`, which we highly unlikely do.

		/// <summary>
		/// An empty <see cref="CString"/> equivalent to <see cref="string.Empty"/>, null terminated for interop.
		/// </summary>				
		public static CString Empty { get; } = new CString(new byte[] { 0x0 });

		[NonSerialized] internal readonly byte[] value;

		#region Construction

		[SecurityCritical]
		private CString(byte[] str) => value = str;

		/// <summary>
		/// CString with a copy of str.
		/// </summary>
		/// <returns>a null terminated CString or null if parameter str == null</returns>
		[SecurityCritical]
		internal static unsafe CString Create(byte* str)
		{
			if (str != null)
			{
				byte* end = str;
				while (*end++ != 0x0) ; // find the end, including the '\0'
				long length = end - str;

				byte[] array = new byte[length];
				Marshal.Copy((IntPtr)str, array, 0, (int)length);
				
				return new CString(array);
			}

			return null;
		}

		/// <summary>
		/// CString with a copy of str, limited to given length.
		/// </summary>
		/// <returns>a null terminated CString or null if parameter str == null</returns>
		[SecurityCritical]
		internal static unsafe CString Create(byte* str, uint length)
		{
			if (str != null)
			{
				byte[] array = new byte[length + 1];
				Marshal.Copy((IntPtr)str, array, 0, (int)length);
				
				return new CString(array);
			}

			return null;
		}

		[SecuritySafeCritical]
		public static unsafe CString Create(string str)
		{
			if (str != null)
			{
				fixed (char* c = str)
				{
					int strLength = str.Length;
					int byteCount = UTF8EncodeLength(c, strLength);
					byte[] bytes = new byte[byteCount + 1]; // zero initializes all, so last byte will be and stay 0x0

					fixed (byte* b = bytes)
						UTF8Encode(b, c, strLength);

					return new CString(bytes);
				}
			}

			return null;
		}

		[SecuritySafeCritical]
		public object Clone() => new CString((byte[])value.Clone());

		#endregion

		#region Accessors

		/// <summary>
		/// Get the length of this string, excluding the null terminator
		/// </summary>
		public int Length => value.Length - 1;

		/// <summary>
		/// Get the length in long form of this string, excluding the null terminator
		/// </summary>
		public long LongLength => value.LongLength - 1;

		/// <summary>
		/// Get the byte at the provided index
		/// </summary>
		/// <param name="index">the position we'd like to retrieve the byte from</param>
		/// <returns>byte representing an ASCII or UTF8 (biggest bit set) character</returns>
		public byte this[int index]
		{
			get => this[(uint)index];
			set => this[(uint)index] = value;
		}

		/// <summary>
		/// Get the byte at the provided index
		/// </summary>
		/// <param name="index">the position we'd like to retrieve the byte from</param>
		/// <returns>byte representing an ASCII or UTF8 (biggest bit set) character</returns>
		public byte this[uint index]
		{
			get
			{
				// unsigned integers only need a single comparison for boundary checks
				// e.g.: (uint)-1 == 4294967295u, so 4294967295u < 20 == false

				if (index < this.value.Length - 1)
					return this.value[index];

				throw new IndexOutOfRangeException();
			}
			set
			{
				if (index < this.value.Length - 1)
					this.value[index] = value;

				throw new IndexOutOfRangeException();
			}
		}

		#endregion

		#region Alteration

		/// <summary>
		/// Retrieves a substring from this instance. The substring starts at a specified character position.
		/// </summary>
		/// <param name="startIndex">start character index</param>
		/// <returns>CString with the requested part of characters of this string</returns>
		/// <exception cref="ArgumentOutOfRangeException"><paramref name="startIndex"/> &lt; 0 or &gt;= Length</exception>
		public CString Substring(int startIndex) => Substring(unchecked((uint)startIndex));

		/// <summary>
		/// Retrieves a substring from this instance. The substring starts at a specified character position.
		/// Unsigned version, requires less checks for the same result.
		/// </summary>
		/// <param name="startIndex">start character index</param>
		/// <returns>CString with the requested part of characters of this string</returns>
		/// <exception cref="ArgumentOutOfRangeException"><paramref name="startIndex"/> &lt; 0 or &gt;= Length</exception>

		[SecuritySafeCritical]
		public CString Substring(uint startIndex)
		{
			uint maxLength = (uint)Length;
			if (startIndex < maxLength)
			{
				uint copyLength = maxLength - startIndex;

				byte[] bytes = new byte[copyLength + 1];
				Buffer.BlockCopy(value, (int)startIndex, bytes, 0, (int)copyLength);

				return new CString(bytes);
			}

			throw new ArgumentOutOfRangeException();
		}

		/// <summary>
		/// Retrieves a substring from this instance. The substring starts at a specified character position and has a specified length.
		/// </summary>
		/// <param name="startIndex">start character index</param>
		/// <param name="length">amount of characters we want in total, can be less. -1 will retrieve all</param>
		/// <returns>CString with the requested part of characters of this string</returns>
		/// <exception cref="ArgumentOutOfRangeException"><paramref name="startIndex"/> &gt;= Length</exception>
		public CString Substring(int startIndex, int length) => Substring(unchecked((uint)startIndex), unchecked((uint)length));

		/// <summary>
		/// Retrieves a substring from this instance. The substring starts at a specified character position and has a specified length.
		/// Unsigned version, requires less checks for the same result.
		/// </summary>
		/// <param name="startIndex">start character index</param>
		/// <param name="length">amount of characters we want in total, can be less.</param>
		/// <returns>CString with the requested part of characters of this string</returns>
		/// <exception cref="ArgumentOutOfRangeException"><paramref name="startIndex"/> &lt; 0 or &gt;= Length</exception>

		[SecuritySafeCritical]
		public CString Substring(uint startIndex, uint length)
		{
			uint maxLength = (uint)Length;
			if (startIndex < maxLength)
			{
				uint copyLength = Math.Min(length, maxLength - startIndex);

				byte[] bytes = new byte[copyLength + 1];
				Buffer.BlockCopy(value, (int)startIndex, bytes, 0, (int)copyLength);

				return new CString(bytes);
			}

			throw new ArgumentOutOfRangeException();
		}

		/// <summary>
		/// Joins/merges all given strings
		/// </summary>
		/// <param name="strings">Strings to join</param>
		/// <returns><see cref="CString"/> containing all given strings concatenated</returns>
		[SecuritySafeCritical]
		public static CString Concat(CString[] strings)
		{
			if (strings != null)
			{
				int size = 1; // null terminator
				for (int i = 0; i < strings.Length; ++i)
				{
					size += strings[i].Length;
				}

				byte[] array = new byte[size];
				int offset = 0;

				for (int i = 0; i < strings.Length; ++i)
				{
					var src = strings[i];

					size = src.Length;
					Buffer.BlockCopy(src.value, 0, array, offset, size);
					offset += size;
				}

				return new CString(array);
			}

			return null;
		}

		[SecuritySafeCritical]
		public static CString operator +(CString left, CString right)
		{
			if (left.value != null)
			{
				if (right.value != null)
				{
					int leftLength = left.value.Length - 1;
					int rightLength = right.value.Length;

					byte[] array = new byte[leftLength + rightLength];
					Array.Copy(left.value, 0, array, 0, leftLength);
					Array.Copy(right.value, 0, array, leftLength, rightLength);
					return new CString(array);
				}

				throw new NullReferenceException("Can't concatenate, right handed CString is null");
			}

			throw new NullReferenceException("Can't concatenate, left handed CString is null");
		}

		#endregion

		#region Comparison

		[SecuritySafeCritical]
		public static unsafe bool operator ==(CString left, CString right)
		{
			if (left == right)
				return true;
			else
				return left.value.SequenceEqual(right.value);
		}

		[SecuritySafeCritical]
		public static unsafe bool operator !=(CString left, CString right)
		{
			if (left != right)
				return true;
			else
				return !left.value.SequenceEqual(right.value);
		}

		public static bool IsNullOrEmpty(CString str) => !(str?.value.Length != 1);
		public override bool Equals(object obj) => obj is CString str && value.Equals(str.value);

		[SecuritySafeCritical]
		public bool Equals(CString other) => this == other || value.Equals(other.value);

		public int CompareTo(object obj) => obj is CString str ? CompareTo(str) : 0;

		public int CompareTo(CString other)
		{
			byte[] otherValues = other.value;
			for (int l = 0, r = 0; l < value.Length && r < otherValues.Length; ++l, ++r)
			{
				int relative = value[l] - otherValues[r];
				if (relative != 0)
					return relative;
			}

			return value.Length - otherValues.Length;
		}

		#endregion

		#region Conversion

		/// <summary>
		/// Converts to a UTF16 string
		/// </summary>
		/// <returns>UTF16 string</returns>
		public override string ToString() => Encoding.UTF8.GetString(value, 0, value.Length - 1);

		/// <summary>
		/// Converts a UTF16 into a UTF8/ASCII string
		/// </summary>
		/// <param name="str">UTF16 string to convert</param>
		/// <returns>UTF8 string</returns>
		public static unsafe CString ToCString(string str) => Create(str);

		/// <summary>
		/// Convert a C# string into a null-terminated c-string
		/// </summary>
		/// <param name="str">UTF16 encoded string (default C#)</param>
		public static implicit operator CString(string str) => Create(str);

		/// <summary>
		/// Convert a null-terminated c-string into a C# string
		/// </summary>
		/// <param name="str">null-terminated c-string</param>
		public static explicit operator string(CString str) => str?.ToString();

		/// <summary>
		/// Copy <see cref="OutString"/> into a null-terminated c-string
		/// </summary>
		/// <param name="str">OutString to copy</param>
		/// <!-- Does put a dependency on OutString -->
		public static implicit operator CString(OutString str) => str.ToCString();

		#endregion

		#region ASCII operations

		/// <summary>
		/// Compares a CString to a C# string only considering ASCII characters (7 bits)
		/// any non 7 bit characters on either side will return false
		/// </summary>
		/// <param name="str">string to compare</param>
		/// <returns>true if both sides have equivalent characters in the ASCII character space</returns>
		public bool CompareASCII(string str)
		{
			return CompareASCII(this, str);
		}

		/// <summary>
		/// Compares a CString to a C# string only considering ASCII characters (7 bits)
		/// any non 7 bit characters on either side will return false
		/// </summary>
		/// <param name="left">CString to compare</param>
		/// <param name="right">string to compare</param>
		/// <returns>true if both sides have equivalent characters in the ASCII character space</returns>
		[SecuritySafeCritical]
		public static unsafe bool CompareASCII(CString left, string right)
		{
			if (left == null && right == null)
				return true;
			else
			{
				fixed (byte* lPin = left.value)
				fixed (char* rPin = right)
				{
					byte* l = lPin, lEnd = lPin + left.value.Length - 1;
					char* r = rPin, rEnd = rPin + right.Length;
					while (l < lEnd && r < rEnd)
					{
						char c1 = *r++;
						if (c1 > 0x7F || c1 != *l++)
							return false;
					}
				}

				return true;
			}
		}

		/// <summary>
		/// Converts a C# string to a CString only accepting ASCII characters (7 bits)
		/// any non 7 bit character will be become the given <paramref name="invalid"/> value
		/// </summary>
		/// <param name="str">string to convert</param>
		/// <param name="invalid">character to insert in case of a non-7 bit character</param>
		/// <returns>CString with only ASCII (7 bit) characters, or null if <paramref name="str"/> == null</returns>
		[SecuritySafeCritical]
		public static unsafe CString ToASCII(string str, byte invalid = (byte)'?')
		{
			fixed (char* src = str)
			{
				if (src != null)
				{
					int byteLength = ASCIIEncodeLength(src, str.Length);
					byte[] bytes = new byte[byteLength + 1];

					fixed (byte* dst = bytes)
						ASCIIEncode(dst, src, str.Length, invalid);

					return new CString(bytes);
				}
			}

			return null;
		}

		#endregion

		#region Encoding algorithms

		[SecuritySafeCritical]
		public override unsafe int GetHashCode()
		{
			fixed (byte* pin = value)
			{
				int hash = 5381;

				byte* c = pin, end = pin + value.Length;
				while (c < end)
					hash += (hash << 5) + (int)*c++;

				return hash;
			}
		}

		[SecurityCritical]
		private static unsafe int UTF8EncodeLength(char* src, int length)
		{
			int byteCount = 0;
			char* end = src + length;
			while (src < end)
			{
				char c1 = *src;
				if (c1 < 0x80)
				{
					// ASCII character: requires 1 byte in UTF8.
					byteCount++;
				}
				else if (c1 < 0x800)
				{
					// Characters in the range [0x80, 0x7FF]: require 2 bytes in UTF8.
					byteCount += 2;
				}
				else if (c1 >= 0xD800 && c1 <= 0xDBFF) // High surrogate
				{
					if (src + 1 < end)
					{
						char c2 = *(src + 1);
						if (c2 >= 0xDC00 && c2 <= 0xDFFF)
						{
							// Valid surrogate pair: requires 4 bytes in UTF8.
							byteCount += 4;
							src++; // Skip the low surrogate as it's processed together with the high surrogate.
						}
						else
						{
							// Invalid surrogate pair: encode using replacement character U+FFFD (3 bytes).
							byteCount += 3;
						}
					}
					else
					{
						// Missing low surrogate: encode using replacement character U+FFFD (3 bytes).
						byteCount += 3;
					}
				}
				else if (c1 >= 0xDC00 && c1 <= 0xDFFF)
				{
					// Lone low surrogate: encode using replacement character U+FFFD (3 bytes).
					byteCount += 3;
				}
				else
				{
					// All other BMP characters (non-surrogate): require 3 bytes in UTF8.
					byteCount += 3;
				}
				src++;
			}
			return byteCount;
		}

		[SecurityCritical]
		private static unsafe int UTF8Encode(byte* dst, char* src, int length)
		{
			byte* start = dst;
			char* end = src + length;
			while (src < end)
			{
				char c1 = *src;
				if (c1 < 0x80)
				{
					// 1-byte ASCII: direct assignment.
					*dst++ = (byte)c1;
				}
				else if (c1 < 0x800)
				{
					// 2-byte encoding for characters in the range [0x80, 0x7FF].
					*dst++ = (byte)(0xC0 | (c1 >> 6));
					*dst++ = (byte)(0x80 | (c1 & 0x3F));
				}
				else if (c1 >= 0xD800 && c1 <= 0xDBFF) // High surrogate
				{
					if (src + 1 < end)
					{
						char c2 = *(src + 1);
						if (c2 >= 0xDC00 && c2 <= 0xDFFF)
						{
							// Valid surrogate pair: compute Unicode code point and encode in 4 bytes.
							int codepoint = 0x10000 + (((c1 - 0xD800) << 10) | (c2 - 0xDC00));
							*dst++ = (byte)(0xF0 | (codepoint >> 18));
							*dst++ = (byte)(0x80 | ((codepoint >> 12) & 0x3F));
							*dst++ = (byte)(0x80 | ((codepoint >> 6) & 0x3F));
							*dst++ = (byte)(0x80 | (codepoint & 0x3F));
							src++; // Skip the low surrogate as it has been processed.
						}
						else
						{
							// Invalid surrogate pair: encode replacement character U+FFFD (0xEF, 0xBF, 0xBD).
							*dst++ = 0xEF;
							*dst++ = 0xBF;
							*dst++ = 0xBD;
						}
					}
					else
					{
						// Missing low surrogate: encode replacement character U+FFFD.
						*dst++ = 0xEF;
						*dst++ = 0xBF;
						*dst++ = 0xBD;
					}
				}
				else if (c1 >= 0xDC00 && c1 <= 0xDFFF)
				{
					// Lone low surrogate: encode replacement character U+FFFD.
					*dst++ = 0xEF;
					*dst++ = 0xBF;
					*dst++ = 0xBD;
				}
				else
				{
					// 3-byte encoding for all other BMP characters.
					*dst++ = (byte)(0xE0 | (c1 >> 12));
					*dst++ = (byte)(0x80 | ((c1 >> 6) & 0x3F));
					*dst++ = (byte)(0x80 | (c1 & 0x3F));
				}
				src++;
			}
			return (int)(dst - start);
		}

		[SecurityCritical]
		private static unsafe int ASCIIEncodeLength(char* src, int length)
		{
			char* c = src, end = src + length;
			length = 0;
			while (c < end)
			{
				length++;

				// increment c to consume the second char and check if that value isn't invalid
				if (*c++ > 0x7FF && *c++ > 0x10FF)
					return length; // error
			}

			return length;
		}

		[SecurityCritical]
		private static unsafe int ASCIIEncode(byte* dst, char* src, int length, byte invalid = (byte)'?')
		{
			byte* s = dst;
			char* c = src, end = src + length;
			length = 0;
			while (c < end)
			{
				length++;

				char c1 = *c++;
				if (c1 <= 0x7F)
					*s++ = (byte)c1; // valid ASCII
				else
				{
					*s++ = invalid; // invalid ASCII

					// increment c to consume the second char and check if that value isn t invalid
					if (c1 > 0x7FF && *c++ > 0x10FF)
						return length; // error
				}
			}

			return length;
		}

		#endregion

		#region Interop/Pinning

		/// <summary>
		/// allows us to directly use `fixed(byte* ptr = cstring)`
		/// </summary>
		/// <returns>reference to first byte</returns>
		internal unsafe ref byte GetPinnableReference() => ref value[0];

		#endregion
	}
}
