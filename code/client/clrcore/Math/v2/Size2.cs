using System;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	/// <summary>
	/// Used for integer sizes with width and height values
	/// </summary>
	[StructLayout(LayoutKind.Explicit)]
	public struct Size2 : IEquatable<Size2>
	{
		[FieldOffset(0)] private int width;
		[FieldOffset(4)] private int height;

		// Overlapping helper fields
		// this will skip C#'s requirement to initialize these fields (they already are by above fields)
		[FieldOffset(0)] private unsafe fixed int values[2];

		/// <summary>
		/// Get or set the width of this element
		/// </summary>
		public int Width { get => width; set => width = value; }

		/// <summary>
		/// Get or set the height of this element
		/// </summary>
		public int Height { get => height; set => height = value; }

		public static readonly Size2 Zero = new Size2(0, 0);
		public static readonly Size2 One = new Size2(1, 1);
		public static readonly Size2 Min = new Size2(int.MinValue, int.MinValue);
		public static readonly Size2 Max = new Size2(int.MaxValue, int.MaxValue);

		/// <summary>
		/// Create a new Size2 with both Width and Height set to <paramref name="value"/>
		/// </summary>
		/// <param name="value">value for both width and height</param>
		public Size2(int value)
		{
			this.width = value;
			this.height = value;
		}

		/// <summary>
		/// Create a new Size2 with specified width and height values
		/// </summary>
		/// <param name="width">width of the Size2</param>
		/// <param name="height">height of the Size2</param>
		public Size2(int width, int height)
		{
			this.width = width;
			this.height = height;
		}

		/// <summary>
		/// Get or set the value at the specified index.
		/// </summary>
		/// <remarks>Does <paramref name="index"/> % 2 internally as it's faster than a boundary check</remarks>
		/// <param name="index">0 for Width and 1 for Height.</param>
		/// <returns>value in either component</returns>
		public int this[int index]
		{
			get => this[(uint)index];
			set => this[(uint)index] = value;
		}

		/// <summary>
		/// Get or set the value at the specified index.
		/// </summary>
		/// <remarks>Does <paramref name="index"/> % 2 internally as it's faster than a boundary check</remarks>
		/// <param name="index">0 for Width and 1 for Height.</param>
		/// <returns>value in either component</returns>
		public unsafe int this[uint index]
		{
#if !OS_LINUX
			get => values[index % 2];
			set => values[index % 2] = value;
#else // compiler wants it pinned for some reason, compiles into a few extra ops
			get { fixed (int* v = values) return v[index % 2]; }
			set { fixed (int* v = values) v[index % 2] = value; }
#endif
		}

		public static Size2 operator +(in Size2 left, in Size2 right) => new Size2(left.width + right.width, left.height + right.height);
		public static Size2 operator -(in Size2 left, in Size2 right) => new Size2(left.width - right.width, left.height - right.height);
		public static Size2 operator *(in Size2 left, in Size2 right) => new Size2(left.width * right.width, left.height * right.height);
		public static Size2 operator /(in Size2 left, in Size2 right) => new Size2(left.width / right.width, left.height / right.height);

		public static Size2 operator +(in Size2 left, int right) => new Size2(left.width + right, left.height + right);
		public static Size2 operator -(in Size2 left, int right) => new Size2(left.width - right, left.height - right);
		public static Size2 operator *(in Size2 left, int right) => new Size2(left.width * right, left.height * right);
		public static Size2 operator /(in Size2 left, int right) => new Size2(left.width / right, left.height / right);

		public static bool operator ==(in Size2 left, in Size2 right) => left.width == right.width && left.height == right.height;
		public static bool operator !=(in Size2 left, in Size2 right) => left.width != right.width || left.height != right.height;

		public bool Equals(Size2 other) => this == other;
		public override bool Equals(object other) => other is Size2 v2 && Equals(v2);

		public override int GetHashCode() => unchecked((width * 397) ^ height);
	}
}
