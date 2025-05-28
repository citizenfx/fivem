#if MONO_V2
using CitizenFX.MsgPack;
using System;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;

#if IS_RDR3
namespace CitizenFX.RedM
#elif IS_FXSERVER
namespace CitizenFX.Server
#else // IS_GTA
namespace CitizenFX.FiveM
#endif // END IS_RDR
{
	public static class MsgPackUtils
	{
		internal delegate TResult TypeDeserializer<out TResult>(ref MsgPackDeserializer arg);

		/// <summary>
		/// Serialize any object using MsgPack serializer.
		/// </summary>
		/// <param name="obj">The object to serialize.</param>
		/// <returns>The serialized object in <see cref="byte"/> array</returns>
		public static byte[] MsgPackSerialize(object obj)
		{
			return MsgPackSerializer.SerializeToByteArray(obj);
		}

		/// <summary>
		/// Try deserializing any <see cref="byte"/> array in the desired type using MsgPack deserializer.
		/// </summary>
		/// <typeparam name="T"/> the type to deserialize to.
		/// <param name="input">The input array</param>
		/// <returns>A <typeparamref name="T"/></returns>
		public unsafe static T MsgPackDeserialize<T>(byte[] input)
		{
			// we don't add try check here, because msgpack deserializer will throw an exception if the input is invalid
			fixed (byte* ptr = input)
			{
				var deserializer = new MsgPackDeserializer(ptr, (ulong)input.Length, "");
				var deleg = MsgPackRegistry.GetOrCreateDeserializer(typeof(T)).CreateDelegate(typeof(TypeDeserializer<T>));
				return ((TypeDeserializer<T>)deleg)(ref deserializer);
			}
		}

		#region Byte encryption
		private static byte[] GenerateIV()
		{
			byte[] rgbIV = new byte[16];
			using (RNGCryptoServiceProvider rng = new RNGCryptoServiceProvider())
				rng.GetBytes(rgbIV);
			return rgbIV;
		}

		private static byte[] EncryptBytes(byte[] data, string strKey)
		{
			byte[] rgbIV = GenerateIV();
			byte[] keyBytes = GenerateSha256Hash(strKey);
			AesManaged aesAlg = new AesManaged
			{
				Key = keyBytes,
				IV = rgbIV
			};

			ICryptoTransform encryptor = aesAlg.CreateEncryptor(aesAlg.Key, aesAlg.IV);

			MemoryStream msEncrypt = new MemoryStream();
			msEncrypt.Write(rgbIV, 0, rgbIV.Length);
			CryptoStream csEncrypt = new CryptoStream(msEncrypt, encryptor, CryptoStreamMode.Write);
			csEncrypt.Write(data, 0, data.Length);
			csEncrypt.FlushFinalBlock();
			return msEncrypt.ToArray();
		}

		private static byte[] DecryptBytes(byte[] data, string strKey)
		{
			byte[] rgbIV = data.Take(16).ToArray(); // Extract the IV from the beginning of the data
			byte[] keyBytes = GenerateSha256Hash(strKey);
			AesManaged aesAlg = new AesManaged
			{
				Key = keyBytes,
				IV = rgbIV
			};

			ICryptoTransform decryptor = aesAlg.CreateDecryptor(aesAlg.Key, aesAlg.IV);

			MemoryStream msDecrypt = new MemoryStream(data.Skip(16).ToArray()); // Skip the IV part
			CryptoStream csDecrypt = new CryptoStream(msDecrypt, decryptor, CryptoStreamMode.Read);
			MemoryStream msDecrypted = new MemoryStream();
			csDecrypt.CopyTo(msDecrypted);

			return msDecrypted.ToArray();
		}
		#endregion

		/// <summary>
		/// Encrypt an object using MsgPack and Sha256 encryption.
		/// </summary>
		/// <param name="obj">The object to encrypt.</param>
		/// <param name="key">The string key to encrypt it.</param>
		/// <exception cref="Exception"></exception>
		/// <returns>An encrypted array of <see cref="byte"/></returns>
		public static byte[] EncryptObject(object obj, string key)
		{
			if (string.IsNullOrWhiteSpace(key))
				throw new Exception("Encryption key cannot be empty!");
			return EncryptBytes(MsgPackSerialize(obj), key);
		}

		/// <summary>
		/// Decrypt an array of <see cref="byte"/> into the desired type.
		/// </summary>
		/// <typeparam name="T"/> the type to deserialize to.
		/// <param name="data">The data to decrypt.</param>
		/// <param name="key">The key to decrypt it (MUST BE THE SAME AS THE ENCRYPTION KEY).</param>
		/// <exception cref="Exception"></exception>
		/// <returns>A <typeparamref name="T"/></returns>
		public static T DecryptObject<T>(byte[] data, string key)
		{
			if (string.IsNullOrWhiteSpace(key))
				throw new Exception("Encryption key cannot be empty!");
			return MsgPackDeserialize<T>(DecryptBytes(data, key));
		}

		/// <summary>
		/// Generate the Sha-256 hash of the given input string.
		/// </summary>
		/// <param name="input">The input string.</param>
		/// <returns>The generated hash in byte[]</returns>
		public static byte[] GenerateSha256Hash(string input)
		{
			SHA256Managed sha256 = new SHA256Managed();
			return sha256.ComputeHash(Encoding.UTF8.GetBytes(input));
		}
	}
}
#endif // END !MONO_V2
