using System;
using System.Collections.Generic;
using System.Dynamic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
	public delegate Coroutine<object> Callback(params object[] args);

	// Can be a struct as it's merely used for for temporary storage
	[SecuritySafeCritical]
	internal struct MsgPackDeserializer
	{
		private unsafe byte* m_ptr;
		private readonly unsafe byte* m_end;
		private readonly string m_netSource;

		private unsafe MsgPackDeserializer(byte* data, ulong size, string netSource)
		{
			m_ptr = data;
			m_end = data + size;
			m_netSource = netSource;
		}

		internal static unsafe object Deserialize(byte[] data, string netSource = null)
		{
			if (data?.Length > 0)
			{
				fixed (byte* dataPtr = data)
					return Deserialize(dataPtr, data.Length, netSource);
			}

			return null;
		}

		internal static unsafe object Deserialize(byte* data, long size, string netSource = null)
		{
			if (data != null && size > 0)
			{
				var deserializer = new MsgPackDeserializer(data, (ulong)size, netSource);
				return deserializer.Deserialize();
			}

			return null;
		}
		
		/// <summary>
		/// Starts deserialization from an array type
		/// </summary>
		/// <param name="data">a byte array</param>
		/// <param name="netSource">from whom came this?</param>
		/// <returns>arguments that can be passed into dynamic delegates</returns>
		public static unsafe object[] DeserializeArray(byte[] data, string netSource = null)
		{
			if (data?.Length > 0)
			{
				fixed (byte* dataPtr = data)
					return DeserializeArray(dataPtr, data.Length, netSource);
			}

			return null;
		}

		/// <summary>
		/// Starts deserialization from an array type
		/// </summary>
		/// <param name="data">ptr to byte data</param>
		/// <param name="size">size of byte data</param>
		/// <param name="netSource">from whom came this?</param>
		/// <returns>arguments that can be passed into dynamic delegates</returns>
		public static unsafe object[] DeserializeArray(byte* data, long size, string netSource = null)
		{
			if (data != null && size > 0)
			{
				var deserializer = new MsgPackDeserializer(data, (ulong)size, netSource);
				return deserializer.DeserializeArray();
			}

			return new object[0];
		}

		private unsafe object[] DeserializeArray()
		{
			int length;
			var type = ReadByte();

			// should start with an array
			if (type >= 0x90 && type < 0xA0)
				length = type % 16;
			else if (type == 0xDC)
				length = ReadUInt16();
			else if (type == 0xDD)
				length = ReadInt32();
			else
				return new object[0];

			object[] array = new object[length];
			for (var i = 0; i < length; ++i)
			{
				array[i] = Deserialize();
			}

			return array;
		}

		private object Deserialize()
		{
			var type = ReadByte();

			if (type < 0xC0)
			{
				if (type < 0x80)
				{
					return type;
				}
				else if (type < 0x90)
				{
					return ReadMap(type % 16u);
				}
				else if (type < 0xA0)
				{
					return ReadObjectArray(type % 16u);
				}

				return ReadString(type % 32u);
			}
			else if (type > 0xDF)
			{
				return type - 256; // fix negative number
			}

			switch (type)
			{
				case 0xC0: return null;

				case 0xC2: return false;
				case 0xC3: return true;

				case 0xC4: return ReadBytes(ReadUInt8());
				case 0xC5: return ReadBytes(ReadUInt16());
				case 0xC6: return ReadBytes(ReadUInt32());

				case 0xC7: return ReadExtraType(ReadUInt8());
				case 0xC8: return ReadExtraType(ReadUInt16());
				case 0xC9: return ReadExtraType(ReadUInt32());

				case 0xCA: return ReadSingle();
				case 0xCB: return ReadDouble();

				case 0xCC: return ReadUInt8();
				case 0xCD: return ReadUInt16();
				case 0xCE: return ReadUInt32();
				case 0xCF: return ReadUInt64();

				case 0xD0: return ReadInt8();
				case 0xD1: return ReadInt16();
				case 0xD2: return ReadInt32();
				case 0xD3: return ReadInt64();

				case 0xD4: return ReadExtraType(1);
				case 0xD5: return ReadExtraType(2);
				case 0xD6: return ReadExtraType(4);
				case 0xD7: return ReadExtraType(8);
				case 0xD8: return ReadExtraType(16);

				case 0xD9: return ReadString(ReadUInt8());
				case 0xDA: return ReadString(ReadUInt16());
				case 0xDB: return ReadString(ReadUInt32());

				case 0xDC: return ReadObjectArray(ReadUInt16());
				case 0xDD: return ReadObjectArray(ReadUInt32());

				case 0xDE: return ReadMap(ReadUInt16());
				case 0xDF: return ReadMap(ReadUInt32());
			}

			throw new InvalidOperationException($"Tried to decode invalid MsgPack type {type}");
		}

		private IDictionary<string, object> ReadMap(uint length)
		{
			var retobject = new ExpandoObject() as IDictionary<string, object>;

			for (var i = 0; i < length; i++)
			{
				var key = Deserialize().ToString();
				var value = Deserialize();

				retobject.Add(key, value);
			}

			return retobject;
		}

		private unsafe byte[] ReadBytes(uint length)
		{
			var ptr = (IntPtr)AdvancePointer(length);

			byte[] retobject = new byte[length];
			Marshal.Copy(ptr, retobject, 0, (int)length);

			return retobject;
		}

		private object[] ReadObjectArray(uint length)
		{
			object[] retobject = new object[length];

			for (var i = 0; i < length; i++)
			{
				retobject[i] = Deserialize();
			}

			return retobject;
		}

		private unsafe float ReadSingle()
		{
			var v = ReadUInt32();
			return *(float*)&v;
		}

		/// <summary>
		/// Read a <see cref="Single"/> stored as little endian, used for custom vectors
		/// </summary>
		private unsafe float ReadSingleLE()
		{
			uint v = *(uint*)AdvancePointer(4);

			if (!BitConverter.IsLittleEndian)
			{
				v = (v >> 16) | (v << 16); // swap adjacent 16-bit blocks
				v = ((v & 0xFF00FF00u) >> 8) | ((v & 0x00FF00FFu) << 8); // swap adjacent 8-bit blocks
			}

			return *(float*)&v;
		}

		private unsafe double ReadDouble()
		{
			var v = ReadUInt64();
			return *(double*)&v;
		}

		private unsafe byte ReadByte()
		{
			byte v = *AdvancePointer(1);
			return v;
		}

		private unsafe byte ReadUInt8() => ReadByte();

		private unsafe ushort ReadUInt16()
		{
			uint v = *(ushort*)AdvancePointer(2);

			if (BitConverter.IsLittleEndian)
				v = (ushort)((v >> 8) | (v << 8)); // swap adjacent 8-bit blocks

			return (ushort)v;
		}

		private unsafe uint ReadUInt32()
		{
			uint v = *(uint*)AdvancePointer(4);

			if (BitConverter.IsLittleEndian)
			{
				v = (v >> 16) | (v << 16); // swap adjacent 16-bit blocks
				v = ((v & 0xFF00FF00u) >> 8) | ((v & 0x00FF00FFu) << 8); // swap adjacent 8-bit blocks
			}

			return v;
		}

		private unsafe ulong ReadUInt64()
		{
			ulong v = *(ulong*)AdvancePointer(8);

			if (BitConverter.IsLittleEndian)
			{				
				v = (v >> 32) | (v << 32); // swap adjacent 32-bit blocks
				v = ((v & 0xFFFF0000FFFF0000u) >> 16) | ((v & 0x0000FFFF0000FFFFu) << 16); // swap adjacent 16-bit blocks
				v = ((v & 0xFF00FF00FF00FF00u) >> 8) | ((v & 0x00FF00FF00FF00FFu) << 8); // swap adjacent 8-bit blocks
			}

			return v;
		}

		private sbyte ReadInt8() => unchecked((sbyte)ReadUInt8());

		private short ReadInt16() => unchecked((short)ReadUInt16());

		private int ReadInt32() => unchecked((int)ReadUInt32());

		private long ReadInt64() => unchecked((long)ReadUInt64());

		private unsafe string ReadString(uint length)
		{
			sbyte* v = (sbyte*)AdvancePointer(length);
			return new string(v, 0, (int)length);
		}

		[SecuritySafeCritical]
		private unsafe CString ReadCString(uint length)
		{
			byte* v = AdvancePointer(length);
			return CString.Create(v, length);
		}

		private object ReadExtraType(uint length)
		{
			var extType = ReadByte();
			switch (extType)
			{
				case 10: // remote funcref
				case 11: // local funcref
					var refFunc = ReadString(length);
					return m_netSource is null
						? _LocalFunction.Create(refFunc)
#if REMOTE_FUNCTION_ENABLED
						: _RemoteFunction.Create(refFunc, m_netSource);
#else
						: null;
#endif
				case 20: // vector2
					return new Vector2(ReadSingleLE(), ReadSingleLE());
				case 21: // vector3
					return new Vector3(ReadSingleLE(), ReadSingleLE(), ReadSingleLE());
				case 22: // vector4
					return new Vector4(ReadSingleLE(), ReadSingleLE(), ReadSingleLE(), ReadSingleLE());
				case 23: // quaternion
					return new Quaternion(ReadSingleLE(), ReadSingleLE(), ReadSingleLE(), ReadSingleLE());
				default: // shouldn't ever happen due to the check above
					throw new InvalidOperationException($"Extension type {extType} not supported.");
			}
		}

		[MethodImpl(MethodImplOptions.AggressiveInlining)]
		private unsafe byte* AdvancePointer(uint amount)
		{
			byte* curPtr = m_ptr;
			m_ptr += amount;
			if (m_ptr > m_end)
			{
				m_ptr -= amount; // reverse damage
				throw new ArgumentException($"MsgPackDeserializer tried to retrieve {amount} bytes while only {m_end - m_ptr} bytes remain");
			}

			return curPtr;
		}
	}
}
