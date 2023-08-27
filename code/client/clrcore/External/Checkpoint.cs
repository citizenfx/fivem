using System;
using CitizenFX.Core.Native;
using System.Security;

#if MONO_V2
using CitizenFX.Core;
using API = CitizenFX.FiveM.Native.Natives;
using InputArgument = CitizenFX.Core.Native.Input.Primitive;

namespace CitizenFX.FiveM
#else
using Color = System.Drawing.Color;

namespace CitizenFX.Core
#endif
{
	public enum CheckpointIcon
	{
		CylinderSingleArrow,
		CylinderDoubleArrow,
		CylinderTripleArrow,
		CylinderCycleArrow,
		CylinderCheckerboard,
		CylinderSingleArrow2,
		CylinderDoubleArrow2,
		CylinderTripleArrow2,
		CylinderCycleArrow2,
		CylinderCheckerboard2,
		RingSingleArrow,
		RingDoubleArrow,
		RingTripleArrow,
		RingCycleArrow,
		RingCheckerboard,
		SingleArrow,
		DoubleArrow,
		TripleArrow,
		CycleArrow,
		Checkerboard,
		CylinderSingleArrow3,
		CylinderDoubleArrow3,
		CylinderTripleArrow3,
		CylinderCycleArrow3,
		CylinderCheckerboard3,
		CylinderSingleArrow4,
		CylinderDoubleArrow4,
		CylinderTripleArrow4,
		CylinderCycleArrow4,
		CylinderCheckerboard4,
		CylinderSingleArrow5,
		CylinderDoubleArrow5,
		CylinderTripleArrow5,
		CylinderCycleArrow5,
		CylinderCheckerboard5,
		RingPlaneUp,
		RingPlaneLeft,
		RingPlaneRight,
		RingPlaneDown,
		Empty,
		Ring,
		Empty2,
		//CylinderCustomShape,
		//CylinderCustomShape2,
		//CylinderCustomShape3,
		Cyclinder = 45,
		Cyclinder2,
		Cyclinder3,
	}
	public enum CheckpointCustomIconStyle
	{
		Number,
		SingleArrow,
		DoubleArrow,
		TripleArrow,
		Ring,
		CycleArrow,
		Ring2,
		RingPointer,
		SegmentedRing,
		Sphere,
		Dollar,
		QuintupleLines,
		BeastIcon,
	}
	public struct CheckpointCustomIcon
	{
		private CheckpointCustomIconStyle _style;
		private byte _number;

		public CheckpointCustomIconStyle Style
		{
			get
			{
				return _style;
			}
			set
			{
				_style = value;
				if (value != CheckpointCustomIconStyle.Number)
				{
					if (_number > 9)
					{
						_number = 0;
					}
				}
			}
		}

		/// <summary>
		/// Initializes a new instance of the <see cref="CheckpointCustomIcon" /> struct.
		/// </summary>
		/// <param name="iconStyle">The icon style.</param>
		/// <param name="iconNumber">The icon number, 
		/// if <paramref name="iconStyle"/> is <see cref="CheckpointCustomIconStyle.Number"/> allowed range is 0 - 99
		/// otherwise allowed range is 0 - 9. </param>
		public CheckpointCustomIcon(CheckpointCustomIconStyle iconStyle, byte iconNumber)
		{
			//initialise them so vs doesnt complain
			_style = CheckpointCustomIconStyle.Number;
			_number = 0;

			Style = iconStyle;
			Number = iconNumber;
		}

		/// <summary>
		/// Gets or sets the number to display inside the icon.
		/// </summary>
		/// <value>
		/// The number.
		/// if <see cref="Style"/> is <see cref="CheckpointCustomIconStyle.Number"/> allowed range is 0 - 99
		/// otherwise allowed range is 0 - 9. 
		/// </value>
		public byte Number
		{
			get
			{
				return _number;
			}
			set
			{
				if(_style == CheckpointCustomIconStyle.Number)
				{
					if(value > 99)
					{
						throw new ArgumentOutOfRangeException("The maximum number value is 99");
					}
					_number = value;
				}
				else
				{
					if(value > 9)
					{
						throw new ArgumentOutOfRangeException("The maximum number value when not using CheckpointCustomIconStyle.Number is 9");
					}
					_number = value;
				}

			}
		}

		private byte getValue()
		{
			if (_style == CheckpointCustomIconStyle.Number)
			{
				return _number;
			}
			return (byte) (90 + (int) _style*10 + _number);
		}


		public static implicit operator InputArgument(CheckpointCustomIcon icon)
		{
			return (InputArgument)(int)icon.getValue();
		}

		public static implicit operator byte(CheckpointCustomIcon icon)
		{
			return icon.getValue();
		}

		public static implicit operator CheckpointCustomIcon(byte value)
		{
			CheckpointCustomIcon c = new CheckpointCustomIcon();
			if(value > 219)
			{
				throw new ArgumentOutOfRangeException("The Range of possible values is 0 to 219");
			}
			if(value < 100)
			{
				c._style = CheckpointCustomIconStyle.Number;
				c._number = value;
			}
			else
			{
				c._style = (CheckpointCustomIconStyle)(value/10 - 9);
				c._number = (byte)(value%10);
			}
			return c;
		}

		public override string ToString()
		{
			return Style.ToString() + Number.ToString();
		}

		public override int GetHashCode()
		{
			return getValue().GetHashCode();
		}
	}
	public sealed class Checkpoint : PoolObject, IEquatable<Checkpoint>
	{
		public Checkpoint(int handle) : base(handle)
		{
		}

		/// <summary>
		/// Gets the memory address of this <see cref="Checkpoint"/>.
		/// </summary>
		public IntPtr MemoryAddress
		{
			get
			{
                // CFX-TODO
                return IntPtr.Zero;
				//return MemoryAccess.GetCheckpointAddress(Handle);
			}
		}

		/// <summary>
		/// Gets or sets the position of this <see cref="Checkpoint"/>.
		/// </summary>
		public Vector3 Position
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 0, Vector3.Zero);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 0, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return Vector3.Zero;
				}
				return MemoryAccess.ReadVector3(memoryAddress);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteVector3(memoryAddress, value);
			}
#endif
		}
		/// <summary>
		/// Gets or sets the position where this <see cref="Checkpoint"/> points to.
		/// </summary>
		public Vector3 TargetPosition
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 16, Vector3.Zero);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 16, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return Vector3.Zero;
				}
				return MemoryAccess.ReadVector3(memoryAddress + 16);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteVector3(memoryAddress + 16, value);
			}
#endif
		}

		/// <summary>
		/// Gets or sets the icon drawn in this <see cref="Checkpoint"/>.
		/// </summary>
		public CheckpointIcon Icon
		{
#if MONO_V2
			[SecuritySafeCritical] get => (CheckpointIcon)MemoryAccess.ReadIfNotNull(MemoryAddress, 56, 0);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 56, (int)value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return (CheckpointIcon)0;
				}
				return (CheckpointIcon)MemoryAccess.ReadInt(memoryAddress + 56);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteInt(memoryAddress + 56, (int)value);
			}
#endif
		}

		public CheckpointCustomIcon CustomIcon
		{
#if MONO_V2
			[SecuritySafeCritical] get => (CheckpointCustomIcon)MemoryAccess.ReadIfNotNull(MemoryAddress, 52, (byte)0);
			[SecuritySafeCritical]
			set
			{
				IntPtr address = MemoryAddress;
				if (address != IntPtr.Zero)
				{
					MemoryAccess.Write(MemoryAddress, 52, (byte)value);
					MemoryAccess.Write(MemoryAddress, 56, (int)42); //sets the icon to a custom icon
				}
			}
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return 0;
				}
				return MemoryAccess.ReadByte(memoryAddress + 52);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteByte(memoryAddress + 52, value);
				MemoryAccess.WriteInt(memoryAddress + 56, 42);//sets the icon to a custom icon
			}
#endif
		}

		/// <summary>
		/// Gets or sets the radius of this <see cref="Checkpoint"/>.
		/// </summary>
		public float Radius
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 60, 0.0f);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 60, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}
				return MemoryAccess.ReadFloat(memoryAddress + 60);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteFloat(memoryAddress + 60, value);
			}
#endif
		}

		/// <summary>
		/// Gets or sets the color of this <see cref="Checkpoint"/>.
		/// </summary>
		public Color Color
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 80, Color.Transparent);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 80, (uint)value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return Color.Transparent;
				}
				return Color.FromArgb(MemoryAccess.ReadInt(memoryAddress + 80));
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteInt(memoryAddress + 80, Color.ToArgb());
			}
#endif
		}
		/// <summary>
		/// Gets or sets the color of the icon in this <see cref="Checkpoint"/>.
		/// </summary>
		public Color IconColor
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 84, Color.Transparent);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 84, (uint)value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return Color.FromArgb(0, 0, 0, 0);
				}
				return Color.FromArgb(MemoryAccess.ReadInt(memoryAddress + 84));
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteInt(memoryAddress + 84, Color.ToArgb());
			}
#endif
		}

		/// <summary>
		/// Gets or sets the near height of the cylinder of this <see cref="Checkpoint"/>.
		/// </summary>
		public float CylinderNearHeight
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 68, 0.0f);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 68, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}
				return MemoryAccess.ReadFloat(memoryAddress + 68);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteFloat(memoryAddress + 68, value);
			}
#endif
		}
		/// <summary>
		/// Gets or sets the far height of the cylinder of this <see cref="Checkpoint"/>.
		/// </summary>
		public float CylinderFarHeight
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 72, 0.0f);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 72, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}
				return MemoryAccess.ReadFloat(memoryAddress + 72);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteFloat(memoryAddress + 72, value);
			}
#endif
		}
		/// <summary>
		/// Gets or sets the radius of the cylinder in this <see cref="Checkpoint"/>.
		/// </summary>
		public float CylinderRadius
		{
#if MONO_V2
			[SecuritySafeCritical] get => MemoryAccess.ReadIfNotNull(MemoryAddress, 76, 0.0f);
			[SecuritySafeCritical] set => MemoryAccess.WriteIfNotNull(MemoryAddress, 76, value);
#else
			get
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return 0.0f;
				}
				return MemoryAccess.ReadFloat(memoryAddress + 76);
			}
			set
			{
				IntPtr memoryAddress = MemoryAddress;
				if (memoryAddress == IntPtr.Zero)
				{
					return;
				}
				MemoryAccess.WriteFloat(memoryAddress + 76, value);
			}
#endif
		}


		/// <summary>
		/// Removes this <see cref="Checkpoint"/>.
		/// </summary>
		public override void Delete()
		{
			API.DeleteCheckpoint(Handle);
		}

		/// <summary>
		/// Determines if this <see cref="Checkpoint"/> exists.
		/// </summary>
		/// <returns><c>true</c> if this <see cref="Checkpoint"/> exists; otherwise, <c>false</c>.</returns>
		public override bool Exists()
		{
			return Handle != 0 && MemoryAddress != IntPtr.Zero;
		}
		/// <summary>
		/// Determines if a specific <see cref="Checkpoint"/> exists.
		/// </summary>
		/// <returns><c>true</c> if the <paramref name="checkpoint"/> exists; otherwise, <c>false</c>.</returns>
		public static bool Exists(Checkpoint checkpoint)
		{
			return !ReferenceEquals(checkpoint, null) && checkpoint.Exists();
		}

		/// <summary>
		/// Determines if a <see cref="Checkpoint"/> refer to the same <see cref="Checkpoint"/> as this <see cref="Checkpoint"/>.
		/// </summary>
		/// <param name="checkpoint">The other <see cref="Checkpoint"/>.</param>
		/// <returns><c>true</c> if the <paramref name="checkpoint"/> is the same checkpoint as this <see cref="Checkpoint"/>; otherwise, <c>false</c>.</returns>
		public bool Equals(Checkpoint checkpoint)
		{
			return !ReferenceEquals(checkpoint, null) && Handle == checkpoint.Handle;
		}
		/// <summary>
		/// Determines if an <see cref="object"/> refer to the same <see cref="Checkpoint"/> as this <see cref="Checkpoint"/>.
		/// </summary>
		/// <param name="obj">The <see cref="object"/> to check.</param>
		/// <returns><c>true</c> if the <paramref name="obj"/> is the same checkpoint as this <see cref="Checkpoint"/>; otherwise, <c>false</c>.</returns>
		public override bool Equals(object obj)
		{
			return !ReferenceEquals(obj, null) && obj.GetType() == GetType() && Equals((Checkpoint)obj);
		}

		public sealed override int GetHashCode()
		{
			return Handle.GetHashCode();
		}

		/// <summary>
		/// Determines if 2 <see cref="Checkpoint"/>s refer to the same checkpoint
		/// </summary>
		/// <param name="left">The left <see cref="Checkpoint"/>.</param>
		/// <param name="right">The right <see cref="Checkpoint"/>.</param>
		/// <returns><c>true</c> if <paramref name="left"/> is the same checkpoint as this <paramref name="right"/>; otherwise, <c>false</c>.</returns>
		public static bool operator ==(Checkpoint left, Checkpoint right)
		{
			return ReferenceEquals(left, null) ? ReferenceEquals(right, null) : left.Equals(right);
		}
		/// <summary>
		/// Determines if 2 <see cref="Checkpoint"/>s don't refer to the same checkpoint
		/// </summary>
		/// <param name="left">The left <see cref="Checkpoint"/>.</param>
		/// <param name="right">The right <see cref="Checkpoint"/>.</param>
		/// <returns><c>true</c> if <paramref name="left"/> is not the same checkpoint as this <paramref name="right"/>; otherwise, <c>false</c>.</returns>
		public static bool operator !=(Checkpoint left, Checkpoint right)
		{
			return !(left == right);
		}
	}
}
