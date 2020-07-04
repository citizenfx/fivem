using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

using MsgPack;
using MsgPack.Serialization;
using System.Security;

namespace CitizenFX.Core
{
    static class MsgPackSerializer
    {
        //private static SerializationContext ms_context = new SerializationContext();

        static MsgPackSerializer()
        {
            //SerializationContext.Default.CompatibilityOptions.PackerCompatibilityOptions = MsgPack.PackerCompatibilityOptions.None; // we want to use ext types, as our unpackers aren't retarded
            //SerializationContext.Default.Serializers.RegisterOverride(new DelegateSerializer());
        }

        private static readonly Type[] WriteTypes = new[] {
            typeof(string), typeof(DateTime), typeof(Enum), 
            typeof(decimal), typeof(Guid),
        };

        public static bool IsSimpleType(this Type type)
        {
            return type.IsPrimitive || WriteTypes.Contains(type);
        }

        public static byte[] Serialize(object obj)
        {
            /*if (obj == null)
            {
                return new byte[] { 0xC0 };
            }

            var serializer = MessagePackSerializer.Get(obj.GetType());

            return serializer.PackSingleObject(obj);*/

            if (obj == null)
            {
                return new byte[] { 0xC0 };
            }

            var stream = new MemoryStream();
            using (var packer = Packer.Create(stream, PackerCompatibilityOptions.None))
            {
                Serialize(obj, packer);

                return stream.ToArray();
            }
        }

        private static void Serialize(object obj, Packer packer)
        {
            if (obj == null)
            {
                packer.PackNull();

                return;
            }

            var type = obj.GetType();

            if (type.IsSimpleType())
            {
                packer.Pack(obj);
            }
			else if (obj is byte[] bytes)
			{
				packer.Pack(bytes);
			}
            else if (obj is IDictionary)
            {
                var dict = (IDictionary)obj;

                packer.PackMapHeader(dict.Count);

                foreach (var key in dict.Keys)
                {
                    Serialize(key, packer);
                    Serialize(dict[key], packer);
                }
            }
            else if (obj is IDictionary<string, object>) // special case for ExpandoObject
            {
                var dict = (IDictionary<string, object>)obj;

                packer.PackMapHeader(dict.Count);

                foreach (var kvp in dict)
                {
                    Serialize(kvp.Key, packer);
                    Serialize(kvp.Value, packer);
                }
            }
            else if (obj is IList)
            {
                var list = (IList)obj;

                packer.PackArrayHeader(list.Count);

                foreach (var item in list)
                {
                    Serialize(item, packer);
                }
            }
            else if (obj is IEnumerable enu)
			{
				var list = new List<object>();

				foreach (var item in enu)
				{
					list.Add(item);
				}

				packer.PackArrayHeader(list.Count);

				list.ForEach(a => Serialize(a, packer));
			}
			else if (obj is IPackable packable)
			{
				packable.PackToMessage(packer, null);
			}
			else if (obj is Delegate deleg)
			{
				var serializer = new DelegateSerializer();
				serializer.PackTo(packer, deleg);
			}
			else if (obj is Vector2 vec2)
			{
				var serializer = new Vector2Serializer();
				serializer.PackTo(packer, vec2);
			}
			else if (obj is Vector3 vec3)
			{
				var serializer = new Vector3Serializer();
				serializer.PackTo(packer, vec3);
			}
			else if (obj is Vector4 vec4)
			{
				var serializer = new Vector4Serializer();
				serializer.PackTo(packer, vec4);
			}
			else if (obj is Quaternion quat)
			{
				var serializer = new QuaternionSerializer();
				serializer.PackTo(packer, quat);
			}
			else
			{
				var properties = type.GetProperties();
				var dict = new Dictionary<string, object>();

				foreach (var property in properties)
				{
					dict[property.Name] = property.GetValue(obj, null);
				}

				Serialize(dict, packer);
			}
		}
    }

	class Vector2Serializer : MessagePackSerializer<Vector2>
	{
		public Vector2Serializer()
			: base(SerializationContext.Default)
		{

		}

		[SecuritySafeCritical]
		protected override void PackToCore(Packer packer, Vector2 vec)
		{
			MemoryStream ms = new MemoryStream();

			Float32Bits.WriteFloat(ms, vec.X);
			Float32Bits.WriteFloat(ms, vec.Y);

			packer.PackExtendedTypeValue(20, ms.ToArray());
		}

		protected override Vector2 UnpackFromCore(Unpacker unpacker)
		{
			Vector2 retValue = new Vector2();

			unpacker.ReadSingle(out retValue.X);
			unpacker.ReadSingle(out retValue.Y);

			return retValue;
		}
	}

	class Vector3Serializer : MessagePackSerializer<Vector3>
	{
		public Vector3Serializer()
			: base(SerializationContext.Default)
		{

		}

		[SecuritySafeCritical]
		protected override void PackToCore(Packer packer, Vector3 vec)
		{
			MemoryStream ms = new MemoryStream();

			Float32Bits.WriteFloat(ms, vec.X);
			Float32Bits.WriteFloat(ms, vec.Y);
			Float32Bits.WriteFloat(ms, vec.Z);

			packer.PackExtendedTypeValue(21, ms.ToArray());
		}

		protected override Vector3 UnpackFromCore(Unpacker unpacker)
		{
			Vector3 retValue = new Vector3();

			unpacker.ReadSingle(out retValue.X);
			unpacker.ReadSingle(out retValue.Y);
			unpacker.ReadSingle(out retValue.Z);

			return retValue;
		}
	}

	class Vector4Serializer : MessagePackSerializer<Vector4>
	{
		public Vector4Serializer()
			: base(SerializationContext.Default)
		{

		}

		[SecuritySafeCritical]
		protected override void PackToCore(Packer packer, Vector4 vec)
		{
			MemoryStream ms = new MemoryStream();

			Float32Bits.WriteFloat(ms, vec.X);
			Float32Bits.WriteFloat(ms, vec.Y);
			Float32Bits.WriteFloat(ms, vec.Z);
			Float32Bits.WriteFloat(ms, vec.W);

			packer.PackExtendedTypeValue(22, ms.ToArray());
		}

		protected override Vector4 UnpackFromCore(Unpacker unpacker)
		{
			Vector4 retValue = new Vector4();

			unpacker.ReadSingle(out retValue.X);
			unpacker.ReadSingle(out retValue.Y);
			unpacker.ReadSingle(out retValue.Z);
			unpacker.ReadSingle(out retValue.W);

			return retValue;
		}
	}

	class QuaternionSerializer : MessagePackSerializer<Quaternion>
	{
		public QuaternionSerializer()
			: base(SerializationContext.Default)
		{

		}

		[SecuritySafeCritical]
		protected override void PackToCore(Packer packer, Quaternion vec)
		{
			MemoryStream ms = new MemoryStream();

			Float32Bits.WriteFloat(ms, vec.X);
			Float32Bits.WriteFloat(ms, vec.Y);
			Float32Bits.WriteFloat(ms, vec.Z);
			Float32Bits.WriteFloat(ms, vec.W);

			packer.PackExtendedTypeValue(23, ms.ToArray());
		}

		protected override Quaternion UnpackFromCore(Unpacker unpacker)
		{
			Quaternion retValue = new Quaternion();

			unpacker.ReadSingle(out retValue.X);
			unpacker.ReadSingle(out retValue.Y);
			unpacker.ReadSingle(out retValue.Z);
			unpacker.ReadSingle(out retValue.W);

			return retValue;
		}
	}

	class DelegateSerializer : MessagePackSerializer<Delegate>
    {
        public DelegateSerializer()
            : base(SerializationContext.Default)
        {

        }

        [SecuritySafeCritical]
        protected override void PackToCore(Packer packer, Delegate objectTree)
        {
            if (objectTree is CallbackDelegate)
            {
                var funcRef = objectTree.Method.DeclaringType?.GetFields(BindingFlags.NonPublic |
                                                                        BindingFlags.Instance |
                                                                        BindingFlags.Public |
                                                                        BindingFlags.Static).FirstOrDefault(a => a.FieldType == typeof(RemoteFunctionReference));

                if (funcRef == null)
                {
                    throw new ArgumentException("The CallbackDelegate does not contain a RemoteFunctionReference capture.");
                }

                var fr = (RemoteFunctionReference)funcRef.GetValue(objectTree.Target);

                packer.PackExtendedTypeValue(10, fr.Duplicate());
            }
            else
            {
                var funcRefDetails = FunctionReference.Create(objectTree);
                var refType = InternalManager.CanonicalizeRef(funcRefDetails.Identifier);

                packer.PackExtendedTypeValue(10, Encoding.UTF8.GetBytes(refType));
            }
        }

        protected override Delegate UnpackFromCore(Unpacker unpacker)
        {
            throw new NotImplementedException();
        }
    }

	/// <summary>
	/// Source: Float32Bits.cs
	/// </summary>
	[StructLayout(LayoutKind.Explicit)]
	internal struct Float32Bits
	{
		/// <summary>
		///		Value as <see cref="Single"/>.
		/// </summary>
		[FieldOffset(0)]
		public readonly float Value;

		/// <summary>
		///		Most significant byte of current endian.
		/// </summary>
		[FieldOffset(0)]
		public readonly Byte Byte0;

		/// <summary>
		///		2nd bit from most significant byte of current endian.
		/// </summary>
		[FieldOffset(1)]
		public readonly Byte Byte1;


		/// <summary>
		///		3rd byte from most significant byte of current endian.
		/// </summary>
		[FieldOffset(2)]
		public readonly Byte Byte2;

		/// <summary>
		///		Least byte of current endian.
		/// </summary>
		[FieldOffset(3)]
		public readonly Byte Byte3;

		/// <summary>
		///		Initializes a new instance of the <see cref="Float32Bits"/> type from specified <see cref="Single"/>.
		/// </summary>
		/// <param name="value">Value of <see cref="Single"/>.</param>
		public Float32Bits(float value)
		{
			this = default(Float32Bits);
			this.Value = value;
		}

		/// <summary>
		///		Initializes a new instance of the <see cref="Float32Bits"/> type from specified <see cref="Byte"/>[] which is big endian.
		/// </summary>
		/// <param name="bigEndianBytes">Array of <see cref="Byte"/> which contains bytes in big endian.</param>
		/// <param name="offset">Offset to read.</param>
		public Float32Bits(byte[] bigEndianBytes, int offset)
		{
#if DEBUG
			Contract.Assert( bigEndianBytes != null, "bigEndianBytes != null" );
			Contract.Assert( bigEndianBytes.Length - offset >= 4, bigEndianBytes.Length + "-" + offset + ">= 4" );
#endif // DEBUG

			this = default(Float32Bits);

			if (BitConverter.IsLittleEndian)
			{
				this.Byte0 = bigEndianBytes[offset + 3];
				this.Byte1 = bigEndianBytes[offset + 2];
				this.Byte2 = bigEndianBytes[offset + 1];
				this.Byte3 = bigEndianBytes[offset];
			}
			else
			{
				this.Byte0 = bigEndianBytes[offset];
				this.Byte1 = bigEndianBytes[offset + 1];
				this.Byte2 = bigEndianBytes[offset + 2];
				this.Byte3 = bigEndianBytes[offset + 3];
			}
		}

		public static void WriteFloat(MemoryStream ms, float value)
		{
			Float32Bits bits = new Float32Bits(value);
			if (BitConverter.IsLittleEndian)
			{
				ms.WriteByte(bits.Byte3);
				ms.WriteByte(bits.Byte2);
				ms.WriteByte(bits.Byte1);
				ms.WriteByte(bits.Byte0);
			}
			else
			{
				ms.WriteByte(bits.Byte0);
				ms.WriteByte(bits.Byte1);
				ms.WriteByte(bits.Byte2);
				ms.WriteByte(bits.Byte3);
			}
		}

		public static float ReadFloat(BinaryReader reader)
		{
			Float32Bits bits = new Float32Bits(reader.ReadBytes(4), 0);
			return bits.Value;
		}
	}
}
