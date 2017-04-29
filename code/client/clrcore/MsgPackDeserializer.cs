using System;
using System.Collections.Generic;
using System.Dynamic;
using System.IO;
using System.Text;

namespace CitizenFX.Core
{
    public delegate object CallbackDelegate(params object[] args);

    internal static class MsgPackDeserializer
    {
        public static object Deserialize(byte[] data)
        {
            var memoryStream = new MemoryStream(data);
            var reader = new BinaryReader(memoryStream);

            return UnpackAny(reader);
        }

        static object UnpackAny(BinaryReader reader)
        {
            var type = reader.ReadByte();
            var unpacker = GetUnpacker(type);

            return unpacker(type, reader);
        }

        static object UnpackMap(BinaryReader reader, int length)
        {
            var retObject = new ExpandoObject() as IDictionary<string, object>;

            for (var i = 0; i < length; i++)
            {
                var key = UnpackAny(reader).ToString();
                var value = UnpackAny(reader);

                retObject[key] = value;
            }

            return retObject;
        }

        static object UnpackArray(BinaryReader reader, int length)
        {
            var retObject = new List<object>();

            for (var i = 0; i < length; i++)
            {
                var value = UnpackAny(reader);

                retObject.Add(value);
            }

            return retObject;
        }

        static object UnpackNil(byte a, BinaryReader reader)
        {
            return null;
        }

        static object UnpackTrue(byte a, BinaryReader reader)
        {
            return true;
        }

        static object UnpackFalse(byte a, BinaryReader reader)
        {
            return false;
        }

        static object UnpackSingle(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(4);

            return BitConverter.ToSingle(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackDouble(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(8);

            return BitConverter.ToDouble(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackFixNumPos(byte a, BinaryReader reader)
        {
            return (int)a;
        }

        static object UnpackUInt8(byte a, BinaryReader reader)
        {
            return reader.ReadByte();
        }

        static object UnpackUInt16(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(2);

            return BitConverter.ToUInt16(new[] { bytes[1], bytes[0] }, 0);
        }

        static object UnpackUInt32(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(4);

            return BitConverter.ToUInt32(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackUInt64(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(8);

            return BitConverter.ToUInt64(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackInt8(byte a, BinaryReader reader)
        {
            return reader.ReadSByte();
        }

        static object UnpackInt16(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(2);

            return BitConverter.ToInt16(new[] { bytes[1], bytes[0] }, 0);
        }

        static object UnpackInt32(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(4);

            return BitConverter.ToInt32(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackInt64(byte a, BinaryReader reader)
        {
            var bytes = reader.ReadBytes(8);

            return BitConverter.ToInt64(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
        }

        static object UnpackFixNumNeg(byte a, BinaryReader reader)
        {
            return a - 256;
        }

        static object UnpackFixStr(byte a, BinaryReader reader)
        {
            var len = a % 32;
            var bytes = reader.ReadBytes(len);

            return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
        }

        static object UnpackString8(byte a, BinaryReader reader)
        {
            var len = reader.ReadByte();
            var bytes = reader.ReadBytes(len);

            return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
        }

        static object UnpackString16(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(2);
            var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

            var bytes = reader.ReadBytes(len);

            return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
        }

        static object UnpackString32(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(4);
            var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

            var bytes = reader.ReadBytes(len);

            return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
        }

        static object UnpackBin8(byte a, BinaryReader reader)
        {
            var len = reader.ReadByte();
            return reader.ReadBytes(len);
        }

        static object UnpackBin16(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(2);
            var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

            return reader.ReadBytes(len);
        }

        static object UnpackBin32(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(4);
            var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

            return reader.ReadBytes(len);
        }

        static object UnpackFixArray(byte a, BinaryReader reader)
        {
            var len = a % 16;

            return UnpackArray(reader, len);
        }

        static object UnpackArray16(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(2);
            var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

            return UnpackArray(reader, len);
        }

        static object UnpackArray32(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(4);
            var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

            return UnpackArray(reader, len);
        }

        static object UnpackFixMap(byte a, BinaryReader reader)
        {
            var len = a % 16;

            return UnpackMap(reader, len);
        }

        static object UnpackMap16(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(2);
            var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

            return UnpackMap(reader, len);
        }

        static object UnpackMap32(byte a, BinaryReader reader)
        {
            var lenBytes = reader.ReadBytes(4);
            var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

            return UnpackMap(reader, len);
        }

        static object UnpackExt8(byte a, BinaryReader reader)
        {
            var length = reader.ReadByte();
            var extType = reader.ReadByte();

            if (extType != 10)
            {
                throw new InvalidOperationException("Only extension type 10 (Citizen funcref) is handled by this class.");
            }

            var funcRefData = reader.ReadBytes(length);
            var remoteFunctionReference = new RemoteFunctionReference(funcRefData);

            return new CallbackDelegate(delegate(object[] args)
            {
                var byteData = MsgPackSerializer.Serialize(args);

                var returnByteData = remoteFunctionReference.InvokeNative(byteData);

                var returnData = Deserialize(returnByteData) as List<object>;

                if (returnData == null || returnData.Count == 0)
                {
                    return null;
                }

                return (returnData)[0];
            });
        }

        private static readonly Dictionary<byte, Func<byte, BinaryReader, object>> ms_unpackMap = new Dictionary<byte, Func<byte, BinaryReader, object>>
        {
            { 0xC0, UnpackNil },
            { 0xC2, UnpackFalse },
            { 0xC3, UnpackTrue },
            { 0xC4, UnpackBin8 },
            { 0xC5, UnpackBin16 },
            { 0xC6, UnpackBin32 },
            { 0xC7, UnpackExt8 },
            { 0xCA, UnpackSingle },
            { 0xCB, UnpackDouble },
            { 0xCC, UnpackUInt8 },
            { 0xCD, UnpackUInt16 },
            { 0xCE, UnpackUInt32 },
            { 0xCF, UnpackUInt64 },
            { 0xD0, UnpackInt8 },
            { 0xD1, UnpackInt16 },
            { 0xD2, UnpackInt32 },
            { 0xD3, UnpackInt64 },
            { 0xD9, UnpackString8 },
            { 0xDA, UnpackString16 },
            { 0xDB, UnpackString32 },
            { 0xDC, UnpackArray16 },
            { 0xDD, UnpackArray32 },
            { 0xDE, UnpackMap16 },
            { 0xDF, UnpackMap32 }
        };

        private static Func<byte, BinaryReader, object> GetUnpacker(byte type)
        {
            if (type < 0xC0)
            {
                if (type < 0x80)
                {
                    return UnpackFixNumPos;
                }

	            if (type < 0x90)
	            {
		            return UnpackFixMap;
	            }

	            if (type < 0xA0)
	            {
		            return UnpackFixArray;
	            }

	            return UnpackFixStr;
            }

	        if (type > 0xDF)
	        {
		        return UnpackFixNumNeg;
	        }

	        return ms_unpackMap[type];
        }
    }
}
