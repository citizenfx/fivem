using System;
using System.Collections.Generic;
using System.Dynamic;
using System.IO;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
	public delegate object CallbackDelegate(params object[] args);

	internal class MsgPackDeserializer
	{
		string NetSource { get; set; }

		public static object Deserialize(byte[] data, string netSource = null)
		{
			var memoryStream = new MemoryStream(data);
			var reader = new BinaryReader(memoryStream);

			var deserializer = new MsgPackDeserializer();
			deserializer.NetSource = netSource;

			return deserializer.UnpackAny(reader);
		}

		object UnpackAny(BinaryReader reader)
		{
			var type = reader.ReadByte();
			var unpacker = GetUnpacker(type);

			return unpacker(type, reader);
		}

		object UnpackMap(BinaryReader reader, int length)
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

		object UnpackArray(BinaryReader reader, int length)
		{
			var retObject = new List<object>();

			for (var i = 0; i < length; i++)
			{
				var value = UnpackAny(reader);

				retObject.Add(value);
			}

			return retObject;
		}

		object UnpackNil(byte a, BinaryReader reader)
		{
			return null;
		}

		object UnpackTrue(byte a, BinaryReader reader)
		{
			return true;
		}

		object UnpackFalse(byte a, BinaryReader reader)
		{
			return false;
		}

		object UnpackSingle(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(4);

			return BitConverter.ToSingle(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackDouble(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(8);

			return BitConverter.ToDouble(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackFixNumPos(byte a, BinaryReader reader)
		{
			return (int)a;
		}

		object UnpackUInt8(byte a, BinaryReader reader)
		{
			return reader.ReadByte();
		}

		object UnpackUInt16(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(2);

			return BitConverter.ToUInt16(new[] { bytes[1], bytes[0] }, 0);
		}

		object UnpackUInt32(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(4);

			return BitConverter.ToUInt32(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackUInt64(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(8);

			return BitConverter.ToUInt64(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackInt8(byte a, BinaryReader reader)
		{
			return reader.ReadSByte();
		}

		object UnpackInt16(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(2);

			return BitConverter.ToInt16(new[] { bytes[1], bytes[0] }, 0);
		}

		object UnpackInt32(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(4);

			return BitConverter.ToInt32(new[] { bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackInt64(byte a, BinaryReader reader)
		{
			var bytes = reader.ReadBytes(8);

			return BitConverter.ToInt64(new[] { bytes[7], bytes[6], bytes[5], bytes[4], bytes[3], bytes[2], bytes[1], bytes[0] }, 0);
		}

		object UnpackFixNumNeg(byte a, BinaryReader reader)
		{
			return a - 256;
		}

		object UnpackFixStr(byte a, BinaryReader reader)
		{
			var len = a % 32;
			var bytes = reader.ReadBytes(len);

			return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
		}

		object UnpackString8(byte a, BinaryReader reader)
		{
			var len = reader.ReadByte();
			var bytes = reader.ReadBytes(len);

			return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
		}

		object UnpackString16(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(2);
			var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

			var bytes = reader.ReadBytes(len);

			return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
		}

		object UnpackString32(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(4);
			var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

			var bytes = reader.ReadBytes(len);

			return Encoding.UTF8.GetString(bytes, 0, bytes.Length);
		}

		object UnpackBin8(byte a, BinaryReader reader)
		{
			var len = reader.ReadByte();
			return reader.ReadBytes(len);
		}

		object UnpackBin16(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(2);
			var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

			return reader.ReadBytes(len);
		}

		object UnpackBin32(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(4);
			var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

			return reader.ReadBytes(len);
		}

		object UnpackFixArray(byte a, BinaryReader reader)
		{
			var len = a % 16;

			return UnpackArray(reader, len);
		}

		object UnpackArray16(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(2);
			var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

			return UnpackArray(reader, len);
		}

		object UnpackArray32(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(4);
			var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

			return UnpackArray(reader, len);
		}

		object UnpackFixMap(byte a, BinaryReader reader)
		{
			var len = a % 16;

			return UnpackMap(reader, len);
		}

		object UnpackMap16(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(2);
			var len = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

			return UnpackMap(reader, len);
		}

		object UnpackMap32(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(4);
			var len = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

			return UnpackMap(reader, len);
		}

		static object CreateRemoteFunctionReference(byte[] funcRefData)
		{
			var remoteFunctionReference = new RemoteFunctionReference(funcRefData);

			return new CallbackDelegate(delegate (object[] args)
			{
				var byteData = MsgPackSerializer.Serialize(args);

				var returnByteData = remoteFunctionReference.InvokeNative(byteData);

				var result = Deserialize(returnByteData) as List<object>;

				if (!(result[0] is bool success))
				{
					return null;
				}

				if (!success)
				{
					var errorMessage = result[1] as string;

					throw new Exception(errorMessage);
				}

				if (!(result[1] is List<object> returnData) || returnData.Count == 0)
				{
					return null;
				}

				if (returnData[0] is IDictionary<string, object> obj)
				{
					if (obj.TryGetValue("__cfx_async_retval", out dynamic asyncRef))
					{
						var tcs = new TaskCompletionSource<object>();

						asyncRef(new Action<dynamic, dynamic>((res, err) =>
						{
							if (err != null && err != false)
							{
								tcs.SetException(new Exception(err.ToString()));
							}
							else
							{
								tcs.SetResult(res[0]);
							}
						}));

						return tcs.Task;
					}
				}

				return (returnData)[0];
			});
		}

		static object CreateNetworkFunctionReference(byte[] funcRefData, string netSource)
		{
			return NetworkFunctionManager.CreateReference(funcRefData, netSource);
		}

		static object CreateFunctionReference(byte[] funcRefData, string netSource)
		{
			return (netSource == null) ? CreateRemoteFunctionReference(funcRefData) : CreateNetworkFunctionReference(funcRefData, netSource);
		}

		object UnpackExt8(byte a, BinaryReader reader)
		{
			var length = reader.ReadByte();

			return UnpackExt(reader, length);
		}

		object UnpackExt16(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(2);
			var length = BitConverter.ToUInt16(new[] { lenBytes[1], lenBytes[0] }, 0);

			return UnpackExt(reader, length);
		}

		object UnpackExt32(byte a, BinaryReader reader)
		{
			var lenBytes = reader.ReadBytes(4);
			var length = BitConverter.ToInt32(new[] { lenBytes[3], lenBytes[2], lenBytes[1], lenBytes[0] }, 0);

			return UnpackExt(reader, length);
		}

		object UnpackFixExt1(byte a, BinaryReader reader) => UnpackExt(reader, 1);
		object UnpackFixExt2(byte a, BinaryReader reader) => UnpackExt(reader, 2);
		object UnpackFixExt4(byte a, BinaryReader reader) => UnpackExt(reader, 4);
		object UnpackFixExt8(byte a, BinaryReader reader) => UnpackExt(reader, 8);
		object UnpackFixExt16(byte a, BinaryReader reader) => UnpackExt(reader, 16);

		private static readonly List<int> ExtTypes = new List<int>()
		{
			10, // funcref
			11, // localfuncref
			20, // vector2
			21, // vector3
			22, // vector4
			23 // quaternion
		};

		private object UnpackExt(BinaryReader reader, int length)
		{
			var extType = reader.ReadByte();

			if (!ExtTypes.Contains(extType))
			{
				throw new InvalidOperationException($"Extension type {extType} not handled.");
			}

			switch (extType)
			{
				case 10: // funcref
				case 11: // localfuncref
					var funcRefData = reader.ReadBytes(length);

					if (extType == 11)
					{
						return CreateRemoteFunctionReference(funcRefData);
					}

					return CreateFunctionReference(funcRefData, NetSource);
				case 20: // vector2
					return new Vector2(reader.ReadSingle(), reader.ReadSingle());
				case 21: // vector3
					return new Vector3(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
				case 22: // vector4
					return new Vector4(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
				case 23: // quaternion
					return new Quaternion(reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle(), reader.ReadSingle());
				default: // shouldn't ever happen due to the check above
					return new object();
			}

		}

		private Func<byte, BinaryReader, object> GetUnpacker(byte type)
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

			switch (type)
			{
				case 0xC0: return UnpackNil;
				case 0xC2: return UnpackFalse;
				case 0xC3: return UnpackTrue;
				case 0xC4: return UnpackBin8;
				case 0xC5: return UnpackBin16;
				case 0xC6: return UnpackBin32;
				case 0xC7: return UnpackExt8;
				case 0xC8: return UnpackExt16;
				case 0xC9: return UnpackExt32;
				case 0xCA: return UnpackSingle;
				case 0xCB: return UnpackDouble;
				case 0xCC: return UnpackUInt8;
				case 0xCD: return UnpackUInt16;
				case 0xCE: return UnpackUInt32;
				case 0xCF: return UnpackUInt64;
				case 0xD0: return UnpackInt8;
				case 0xD1: return UnpackInt16;
				case 0xD2: return UnpackInt32;
				case 0xD3: return UnpackInt64;
				case 0xD4: return UnpackFixExt1;
				case 0xD5: return UnpackFixExt2;
				case 0xD6: return UnpackFixExt4;
				case 0xD7: return UnpackFixExt8;
				case 0xD8: return UnpackFixExt16;
				case 0xD9: return UnpackString8;
				case 0xDA: return UnpackString16;
				case 0xDB: return UnpackString32;
				case 0xDC: return UnpackArray16;
				case 0xDD: return UnpackArray32;
				case 0xDE: return UnpackMap16;
				case 0xDF: return UnpackMap32;
			}

			throw new InvalidOperationException($"Tried to decode invalid MsgPack type {type}");
		}
	}
}
