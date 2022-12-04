using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[StructLayout(LayoutKind.Explicit)]
	public struct Color : IEquatable<Color>
	{

		/// <summary>
		/// Get the blue channel of this color
		/// </summary>
		[FieldOffset(0)] public byte B;

		/// <summary>
		/// Get the green channel of this color
		/// </summary>
		[FieldOffset(1)] public byte G;

		/// <summary>
		/// Get the red channel of this color
		/// </summary>
		[FieldOffset(2)] public byte R;

		/// <summary>
		/// Get the alpha/translucency channel of this color
		/// </summary>
		[FieldOffset(3)] public byte A;

		// Overlapping helper fields
		// this will skip C#'s requirement to initialize these fields (they already are by above fields)
		[FieldOffset(0)] private unsafe fixed byte values[4];
		[FieldOffset(0)] private unsafe fixed uint argb[1];

		public readonly static Color Black = new Color(0xFF000000);
		public readonly static Color White = new Color(0xFFFFFFFF);

		public readonly static Color Red = new Color(0xFFFF0000);
		public readonly static Color Green = new Color(0xFF00FF00);
		public readonly static Color Blue = new Color(0xFF0000FF);
		public readonly static Color Transparent = new Color(0x00000000);

		/// <summary>
		/// Create a fully opaque color by individual given channels
		/// </summary>
		/// <param name="r">red</param>
		/// <param name="g">green</param>
		/// <param name="b">blue</param>
		public unsafe Color(byte r, byte g, byte b)
		{
			A = 0xFF;
			R = r;
			G = g;
			B = b;
		}

		/// <summary>
		/// Create a color by individual given channels
		/// </summary>
		/// <param name="a">alpha/translucency</param>
		/// <param name="r">red</param>
		/// <param name="g">green</param>
		/// <param name="b">blue</param>
		public unsafe Color(byte a, byte r, byte g, byte b)
		{
			A = a;
			R = r;
			G = g;
			B = b;
		}

		/// <summary>
		/// Create a color with an unsinged integer
		/// </summary>
		/// <param name="argb">argb values</param>
		public unsafe Color(uint argb) : this() => this.argb[0] = argb;

		/// <summary>
		/// Create a color with a signed integer
		/// </summary>
		/// <param name="argb">argb values</param>
		public unsafe Color(int argb) : this() => this.argb[0] = (uint)argb;

		/// <summary>
		/// Get or set the channel at the specified index
		/// </summary>
		/// <remarks>Does <paramref name="index"/> % 4 internally as it's faster than a boundary check</remarks>
		/// <param name="index">channel to get the value for</param>
		/// <returns></returns>
		public unsafe byte this[int index]
		{
			get => this[(uint)index];
			set => this[(uint)index] = value;
		}

		/// <summary>
		/// Get or set the channel at the specified index
		/// </summary>
		/// <remarks>Does <paramref name="index"/> % 4 internally as it's faster than a boundary check</remarks>
		/// <param name="index">channel to get the value for</param>
		/// <returns></returns>
		public unsafe byte this[uint index]
		{
#if !OS_LINUX
			get => values[index % 4];
			set => values[index % 4] = value;
#else // compiler wants it pinned for some reason, compiles into a few extra ops
			get { fixed (byte* v = values) return v[index % 4]; }
			set { fixed (byte* v = values) v[index % 4] = value; }
#endif
		}

		public static unsafe explicit operator uint(in Color color) => color.argb[0];
		public static unsafe explicit operator int(in Color color) => (int)color.argb[0];

		[Obsolete("use `new Color(byte, byte, byte, byte)` instead")]
		internal static Color FromArgb(byte a, byte r, byte g, byte b) => new Color(a, r, g, b);

		[Obsolete("use `new Color(byte, byte, byte)` instead")]
		internal static Color FromArgb(int r, int g, int b) => new Color((byte)r, (byte)g, (byte)b);

		[Obsolete("use `new Color(int)` instead")]
		internal static Color FromArgb(int argb) => new Color(argb);

		[Obsolete("use `(int)color` instead")]
#if !OS_LINUX
		internal unsafe int ToArgb() => (int)argb[0];

		public override unsafe string ToString() => $"Color({values[0]}, {values[1]}, {values[2]}, {values[3]})";
#else // compiler wants it pinned for some reason, compiles into a few extra ops
		internal unsafe int ToArgb() { fixed(uint* p = argb) return (int)p[0]; }

		public override unsafe string ToString() { fixed(uint* p = argb) return $"Color({p[0]}, {p[1]}, {p[2]}, {p[3]})"; }
#endif

		public unsafe bool Equals(Color other) => (uint)this == (uint)other;
	}
}
