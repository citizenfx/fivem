using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Security;
using System.Reflection;
using System.Text;

using CitizenFX.Core.Native;
using System.Reflection.Metadata;
using System.Security.Policy;

#if IS_FXSERVER
using ContextType = CitizenFX.Core.fxScriptContext;
#else
using ContextType = CitizenFX.Core.RageScriptContext;
#endif

namespace CitizenFX.Core
{
	public static class ScriptContext
	{
		internal static readonly ConcurrentQueue<GCHandle> s_gcHandles = new ConcurrentQueue<GCHandle>();

		private static byte[] s_stringHeap = new byte[1024 * 256];
		private static int s_stringHeapPosition = 0;
		private static GCHandle s_stringHeapHandle;
		private static IntPtr s_stringHeapHandlePtr;

		private static List<IntPtr> s_cleanUp = new List<IntPtr>();

		[SecuritySafeCritical]
		static ScriptContext()
		{
			s_stringHeapHandle = GCHandle.Alloc(s_stringHeap, GCHandleType.Pinned);
			s_stringHeapHandlePtr = s_stringHeapHandle.AddrOfPinnedObject();
		}

		internal static void CleanUp()
		{
			s_stringHeapPosition = 0;

			while (s_gcHandles.TryDequeue(out GCHandle handle))
			{
				handle.Free();
			}
		}

		internal static unsafe ByteArray SerializeObject(object v)
		{
			var argsSerialized = MsgPackSerializer.Serialize(v);

			var handle = GCHandle.Alloc(argsSerialized, GCHandleType.Pinned);
			s_gcHandles.Enqueue(handle);
			return new ByteArray((byte*)handle.AddrOfPinnedObject(), unchecked((ulong)argsSerialized.LongLength));
		}

		[SecurityCritical]
		internal static IntPtr StringToCString(string str)
		{
			if (str != null)
			{
				int heapPtr = s_stringHeapPosition;
				int newLen = Encoding.UTF8.GetByteCount(str);

				// worst-case UTF8 length
				if (heapPtr + newLen < 1024 * 64 - 1)
				{
					Encoding.UTF8.GetBytes(str, 0, str.Length, s_stringHeap, heapPtr);
					s_stringHeap[heapPtr + newLen] = 0;
					s_stringHeapPosition += newLen + 1;

					return s_stringHeapHandlePtr + heapPtr;
				}
				else // too long, doesn't fit, allocate traditionally
				{
					byte[] buffer = new byte[newLen + 1];
					Encoding.UTF8.GetBytes(str, 0, str.Length, buffer, 0);

					var handle = GCHandle.Alloc(buffer, GCHandleType.Pinned);
					s_gcHandles.Enqueue(handle);

					return handle.AddrOfPinnedObject();
				}
			}

			return IntPtr.Zero;
		}

		[Flags]
		internal enum PASFlags : byte
		{
			FUNC_REF = 0x80,
			STRING = 0x40,
			OBJECT = 0x20,

			PRIMITIVES = 0x3 // the amount of 64 bit slots we may read from the start as value types
		}

#region Context Getting/Reading
		[SecurityCritical]
		internal unsafe static object GetResult(Type type, ulong* ptr, ulong hash)
		{
#if NATIVE_PTR_CHECKS_INCLUDE
			PointerArgumentSafety.s_nativeInfo.TryGetValue(hash, out byte retSafetyInfo); // sets retSafetyInfo to default(T)/zero if it's not found
#else
			byte retSafetyInfo = 0xFF; // all checks pass
#endif

			if (type.IsValueType)
			{
				if (type.IsEnum)
				{
					if ((retSafetyInfo & (byte)PASFlags.PRIMITIVES) > 0)
						return Enum.ToObject(type, *ptr);
				}
				else if (type == typeof(Vector3))
				{
					if ((retSafetyInfo & (byte)PASFlags.PRIMITIVES) > 2)
						return new Vector3(*(float*)&ptr[0], *(float*)&ptr[1], *(float*)&ptr[2]); // 8 bytes stride
				}
				else if (Marshal.SizeOf(type) <= 8)
				{
					if ((retSafetyInfo & (byte)PASFlags.PRIMITIVES) > 0)
						return Marshal.PtrToStructure((IntPtr)ptr, type);
				}

				return Activator.CreateInstance(type); // default
			}
			else
			{
				if (type == typeof(string))
				{
					if ((retSafetyInfo & (byte)PASFlags.STRING) != 0)
						return Marshal.PtrToStringAnsi((IntPtr)ptr[0]);
				}
				else if (type == typeof(object))
				{
					if ((retSafetyInfo & (byte)PASFlags.OBJECT) != 0)
						return MsgPackDeserializer.Deserialize((byte*)ptr[0], *(long*)&ptr[1]);
				}
				else if (type == typeof(Callback))
				{
					if ((retSafetyInfo & (byte)PASFlags.FUNC_REF) != 0)
						return _LocalFunction.Create(Marshal.PtrToStringAnsi((IntPtr)ptr[0]));
				}

				return null;
			}
		}

		internal unsafe static T GetResultPrimitive<T>(Type type, ulong* ptr, ulong hash) where T : unmanaged
		{
#if NATIVE_PTR_CHECKS_INCLUDE
			PointerArgumentSafety.s_nativeInfo.TryGetValue(hash, out byte retSafetyInfo); // sets retSafetyInfo to default(T)/zero if it's not found
#else
			byte retSafetyInfo = 0xFF; // all checks pass
#endif
			if ((retSafetyInfo & (byte)PASFlags.PRIMITIVES) > 0)
				return *(T*)ptr;
			else
				return default(T);
		}

		#endregion

		#region Native Invoking

		[SecurityCritical, SkipLocalsInit, MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static unsafe void InvokeNative(ref UIntPtr handle, ulong hash, ulong* data, int size)
		{
			ContextType* ctx = stackalloc ContextType[1];
			ctx->Initialize(data, size);

			if (handle == UIntPtr.Zero) handle = ScriptInterface.GetNative(hash);
			ScriptInterface.InvokeNative(handle, ref *ctx, hash);
		}

		[SecurityCritical, SkipLocalsInit, MethodImpl(MethodImplOptions.AggressiveInlining)]
		internal static unsafe void InvokeNativeOutVec3(ref UIntPtr handle, ulong hash, ulong* data, int size)
		{
			ContextType* ctx = stackalloc ContextType[1];
			ctx->Initialize(data, size);

			if (handle == UIntPtr.Zero) handle = ScriptInterface.GetNative(hash);
			ScriptInterface.InvokeNative(handle, ref *ctx, hash);
#if !IS_FXSERVER
			RageScriptContext.CopyVectorData(ctx);
#endif
		}

		#endregion
	}
	internal struct ByteArray
	{
		public unsafe byte* ptr;
		public ulong size;

		internal unsafe ByteArray(byte* ptr, ulong size)
		{
			this.ptr = ptr;
			this.size = size;
		}
	}

	#region Utils

	[SecuritySafeCritical]
	internal static class UnmanagedTypeInfo<T>
	{
		/// <summary>
		/// Get unmanaged type size, this will actually request the size from native, then we cache it
		/// </summary>
		/// <returns>unmanaged size of T</returns>
		public static int Size { get; } = Marshal.SizeOf(typeof(T));
	}

	#endregion
}
