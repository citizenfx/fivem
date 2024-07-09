using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using MsgPack;
using MsgPack.Serialization;
using System.Security;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
	[SecuritySafeCritical]
	internal static class MsgPackSerializer
	{
		internal readonly static ByteArray nullResult;

		[ThreadStatic]
		private static bool ts_remote;

		[SecuritySafeCritical]
		static unsafe MsgPackSerializer()
		{
			byte* nullData = (byte*)Marshal.AllocCoTaskMem(1);
			*nullData = 0xC0;
			nullResult = new ByteArray(nullData, 1);
		}

		internal static byte[] Serialize(object obj, bool remote = false)
		{
			if (obj is object) // not null
			{
				ts_remote = remote;

				var stream = new MemoryStream(256);
				using (var packer = Packer.Create(stream, PackerCompatibilityOptions.None))
				{
					Serialize(obj, packer);
					return stream.ToArray(); // other runtimes read the full length we give them, so shrink the buffer (copy)
					//return stream.GetBuffer();
				}
			}

			return new byte[] { 0xC0 }; // null
		}

		/// <summary>
		/// Experimental, uses the CoTask memory instead of a managed byte[]
		/// </summary>
		/// <param name="obj"></param>
		/// <param name="remote"></param>
		/// <returns></returns>
		internal static ByteArray SerializeCoTaskMemory(object obj, bool remote = false)
		{
			if (obj is object) // not null
			{
				ts_remote = remote;

				var stream = new CoTaskMemoryStream(256);
				using (var packer = Packer.Create(stream, PackerCompatibilityOptions.None))
				{
					Serialize(obj, packer);
					//return stream.ToArray();
					return stream.TakeBuffer();
				}
			}

			return nullResult;
		}

		[SecuritySafeCritical]
		private static void Serialize(object obj, Packer packer)
		{
			// This msgpack packer is heaven if you love checking all arguments *multiple* times and throwing exceptions all over the place (hence the checking it multiple times), lol
			// SerializationContext.Default.GetSerializer<T>().PackTo(packer, obj) == packer.Pack<T>(obj) without all the hoops and checks
			void Pack<T>(Packer p, T o) => SerializationContext.Default.GetSerializer<T>().PackTo(p, o);

			if (obj is null)
			{
				packer.PackNull();
				return;
			}

			var type = obj.GetType();

			if (type.IsPrimitive
				|| type == typeof(string) || type == typeof(decimal)
				|| type == typeof(DateTime) || type == typeof(Guid))
			{
				Pack(packer, obj);
				return;
			}
			else if (type.IsEnum)
			{
				// enums in C# may only be of an integer types
				Pack(packer, Convert.ToUInt64(obj));
				return;
			}
			else if (type.IsValueType) // structs in C#
			{
				switch (obj)
				{
					case Vector2 vec2:
						unsafe
						{
							byte[] data = new byte[2 * sizeof(float)];
							fixed (byte* ptr = data)
							{
								*((float*)ptr) = vec2.X;
								*((float*)ptr + 1) = vec2.Y;
							}
							packer.PackExtendedTypeValue(20, data);
						}
						return;

					case Vector3 vec3:
						unsafe
						{
							byte[] data = new byte[3 * sizeof(float)];
							fixed (byte* ptr = data)
							{
								*((float*)ptr) = vec3.X;
								*((float*)ptr + 1) = vec3.Y;
								*((float*)ptr + 2) = vec3.Z;
							}
							packer.PackExtendedTypeValue(21, data);
						}
						return;

					case Vector4 vec4:
						unsafe
						{
							byte[] data = new byte[4 * sizeof(float)];
							fixed (byte* ptr = data)
							{
								*((float*)ptr) = vec4.X;
								*((float*)ptr + 1) = vec4.Y;
								*((float*)ptr + 2) = vec4.Z;
								*((float*)ptr + 3) = vec4.W;
							}
							packer.PackExtendedTypeValue(22, data);
						}
						return;

					case Quaternion quat:
						unsafe
						{
							byte[] data = new byte[4 * sizeof(float)];
							fixed (byte* ptr = data)
							{
								*((float*)ptr) = quat.X;
								*((float*)ptr + 1) = quat.Y;
								*((float*)ptr + 2) = quat.Z;
								*((float*)ptr + 3) = quat.W;
							}
							packer.PackExtendedTypeValue(23, data);
						}
						return;
				}
			}
			else if(obj is IEnumerable enumerable) // this includes any container like, arrays and generic ones (ICollection)
			{
				switch (enumerable)
				{
					case byte[] bytes:
						Pack(packer, bytes);
						return;

					case IDictionary<string, object> dictStringObject: // more common than below, also faster iteration
						{
							packer.PackMapHeader(dictStringObject.Count);
							foreach (var kvp in dictStringObject)
							{
								Serialize(kvp.Key, packer);
								Serialize(kvp.Value, packer);
							}
						}
						return;

					case IDictionary dict: // less common than above, will handle all types of dictionaries. 1.3 (30%) times slower than above
						{
							packer.PackMapHeader(dict.Count);
							foreach (DictionaryEntry kvp in dict)
							{
								Serialize(kvp.Key, packer);
								Serialize(kvp.Value, packer);
							}
						}
						return;

					case IList array: // any array like, including T[] and List<T>
						{
							packer.PackArrayHeader(array.Count);
							for (int i = 0; i < array.Count; ++i)
							{
								Serialize(array[i], packer);
							}
						}
						return;

					default: // unknown/undefined size, so go through them
						{
							var list = new List<object>();
							foreach (var item in enumerable)
							{
								list.Add(item);
							}

							packer.PackArrayHeader(list.Count);
							for (int i = 0; i < list.Count; ++i)
							{
								Serialize(list[i], packer);
							}
						}
						return;
				}
			}

			switch (obj)
			{
				case IPackable packable:
					packable.PackToMessage(packer, null);
					return;

				case Delegate deleg:
					PackDelegate(packer, deleg);
					return;
			}

			// no suitable conversion found, serialize all public properties into a dictionary
			// TODO: check if this is really what we want
			var dictionary = new Dictionary<string, object>();

			var properties = type.GetProperties();
			for (int i = 0; i < properties.Length; ++i)
			{
				var property = properties[i];
				dictionary[property.Name] = property.GetValue(obj, null);
			}

			/*
			// fields as well?
			var fields = type.GetFields();
			for (int i = 0; i < fields.Length; ++i)
			{
				var field = fields[i];
				dictionary[field.Name] = field.GetValue(obj);
			}*/

			Serialize(dictionary, packer);
		}

		[SecuritySafeCritical]
		private static void PackDelegate(Packer packer, in Delegate deleg)
		{
			/*if (deleg is Callback)
			{
				var funcRef = deleg.Method.DeclaringType?.GetFields(
					BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Instance | BindingFlags.Static
					).FirstOrDefault(a => a.FieldType == typeof(RemoteFunction));

				if (funcRef != null)
				{
					var remoteFunction = (RemoteFunction)funcRef.GetValue(deleg.Target);

					DevDebug.WriteLine($"MsgPackSerializer:WriteDelegate({deleg}) {deleg.Method} {deleg.GetHashCode()} as ?");
					packer.PackExtendedTypeValue(10, remoteFunction.Duplicate());
				}
				else
				{
					throw new ArgumentException("The CallbackDelegate does not contain a RemoteFunction capture.");
				}

				return;
			}*/

			var dynFunc = deleg as DynFunc ?? Func.Create(deleg);

			if (!ts_remote)
			{
				var refType = ReferenceFunctionManager.Create(dynFunc);
				packer.PackExtendedTypeValue(11, refType.Value);
			}
			else
			{
#if REMOTE_FUNCTION_ENABLED
				ulong callbackId;

				if (deleg.Target is _RemoteHandler _pf)
				{
					callbackId = _pf.m_id;
				}
				else
				{
					callbackId = ExternalsManager.RegisterRemoteFunction(deleg.Method.ReturnType, new DynFunc(args =>
						args.Length == 1 || args[1] == null ? dynFunc(args[0]) : null));
				}

				packer.PackExtendedTypeValue(10, Encoding.UTF8.GetBytes(callbackId.ToString()));
#endif
				packer.PackNull();
			}
		}
	}

	/// <summary>
	/// Experimental memory stream
	/// </summary>
	[SecuritySafeCritical]
	internal class CoTaskMemoryStream : Stream
	{
		private unsafe IntPtr buffer;
		private int capacity;
		private int position;
		private int length;

		private bool canRead;
		private bool canWrite;

		public override bool CanRead => canRead;

		public override bool CanSeek => false;

		public override bool CanWrite => canWrite;

		public override long Length => length;

		public override long Position
		{
			get => position;
			set
			{
				if (value < Length)
					position = (int)value & int.MaxValue; // makes sure it's always >= 0
				else
					throw new NotSupportedException();
			}
		}

		public unsafe CoTaskMemoryStream(byte* ptr, int size)
		{
			buffer = (IntPtr)ptr;
			capacity = length = size;
			position = 0;
			canRead = true;
			canWrite = false;
		}

		public unsafe CoTaskMemoryStream(int initialSize = 64)
		{
			buffer = Marshal.AllocCoTaskMem(initialSize);
			capacity = initialSize;
			position = length = 0;
			canRead = false;
			canWrite = true;
		}

		[SecurityCritical]
		public unsafe ByteArray TakeBuffer()
		{
			var result = new ByteArray((byte*)buffer, (ulong)length);

			buffer = IntPtr.Zero;
			canRead = canWrite = false;
			capacity = length = position = 0;

			return result;
		}

		[SecuritySafeCritical]
		public unsafe override void SetLength(long value)
		{
			int size = (int)value;

			if (size < capacity)
				length = size;
			else if (value > capacity)
				EnsureCapacity(size);
		}

		private void EnsureCapacity(int newCapacity)
		{
			// same setup as MemoryStream
			int num = newCapacity;
			if (num < 256)
				num = 256;

			if (num < capacity * 2)
				num = capacity * 2;

			if ((uint)(capacity * 2) > 2147483591u)
				num = ((newCapacity > 2147483591) ? newCapacity : 2147483591);

			buffer = Marshal.ReAllocCoTaskMem(buffer, num);
			capacity = newCapacity;
		}

		[SecuritySafeCritical]
		public unsafe override void Write(byte[] buffer, int offset, int count)
		{
			if (position + count > capacity)
				EnsureCapacity(position + count);

			Marshal.Copy(buffer, offset, this.buffer + position, count);
		}

		public override void Flush() { }

		public override long Seek(long offset, SeekOrigin origin)
		{
			throw new NotImplementedException();
		}

		[SecuritySafeCritical]
		public override int Read(byte[] buffer, int offset, int count)
		{
			int length = Math.Min(count, this.length - position);
			Marshal.Copy(this.buffer + position, buffer, offset, length);
			return length;
		}
	}
}
