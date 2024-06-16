using CitizenFX.MsgPack;
using System;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;

namespace CitizenFX.Core.Native
{
	public struct N64
	{
		internal ulong value;

		public static unsafe ulong Val(int v) => *(uint*)&v;
		public static unsafe ulong Val(uint v) => v;
		public static unsafe ulong Val(long v) => *(ulong*)&v;
		public static unsafe ulong Val(ulong v) => v;
		public static unsafe ulong Val(float v) => *(uint*)&v;
		public static unsafe ulong Val(double v) => *(ulong*)&v;
		public static unsafe ulong Val(bool v) => *(byte*)&v;

		public static unsafe bool To_bool(ulong v) => *(bool*)&v;
		public static unsafe int To_int(ulong v) => *(int*)&v;
		public static unsafe uint To_uint(ulong v) => *(uint*)&v;
		public static unsafe float To_float(ulong v) => *(float*)&v;
		public static unsafe double To_double(ulong v) => *(float*)&v;
		public static unsafe long To_long(ulong v) => *(long*)&v;
		public static unsafe ulong To_ulong(ulong v) => v;


		public static unsafe string To_string(string v) => v;
		public static unsafe object To_object(object v) => v;
		public static unsafe Vector3 To_Vector3(in Vector3 v) => v;

		[SecurityCritical] internal static unsafe bool To_bool(ulong* v) => *(bool*)v;
		[SecurityCritical] internal static unsafe int To_int(ulong* v) => *(int*)v;
		[SecurityCritical] internal static unsafe uint To_uint(ulong* v) => *(uint*)v;
		[SecurityCritical] internal static unsafe float To_float(ulong* v) => *(float*)v;
		[SecurityCritical] internal static unsafe double To_double(ulong* v) => *(double*)v;
		[SecurityCritical] internal static unsafe long To_long(ulong* v) => *(long*)v;
		[SecurityCritical] internal static unsafe ulong To_ulong(ulong* v) => *v;

		[SecurityCritical] internal static unsafe string ToString(ulong* v) => Marshal.PtrToStringAnsi((IntPtr)(byte*)v[0]);
		[SecurityCritical] internal static unsafe object ToObject(ulong* v) => MsgPackDeserializer.DeserializeAsObject((byte*)v[0], unchecked((long)v[1]));
		[SecurityCritical] internal static unsafe Vector3 To_Vector3(ulong* v) => (Vector3)(*(NativeVector3*)v);
	}

	[StructLayout(LayoutKind.Explicit)]
	internal struct NativeVector3
	{
		[FieldOffset(0)] public float x;
		[FieldOffset(8)] public float y;
		[FieldOffset(16)] public float z;

		public NativeVector3(Vector3 copy)
		{
			x = copy.X;
			y = copy.Y;
			z = copy.Z;
		}

		public static implicit operator NativeVector3(Vector3 v) => new NativeVector3(v);
		public static implicit operator Vector3(NativeVector3 self) => new Vector3(self.x, self.y, self.z);
	}

	public readonly ref struct InFunc
	{
		internal readonly byte[] value;
		
		internal InFunc(byte[] funcRef) => value = funcRef;
		public InFunc(MsgPackFunc del) => value = ReferenceFunctionManager.Create(del).Value;
		public InFunc(Delegate del) : this(MsgPackDeserializer.CreateDelegate(del)) { }

		public static implicit operator InFunc(Delegate func) => new InFunc(func);
		public static implicit operator InFunc(MsgPackFunc func) => new InFunc(func);
	}

	public readonly ref struct InPacket
	{
		internal readonly byte[] value;
		public InPacket(object obj) => value = MsgPackSerializer.SerializeToByteArray(obj);
		public static InPacket Serialize(object obj) => new InPacket(obj);
		public static implicit operator InPacket(object[] obj) => Serialize(obj);
	}

	[SecuritySafeCritical]
	public readonly ref struct OutPacket
	{
#pragma warning disable 0649 // this type is reinterpreted i.e.: (OutPacket*)ptr
		private unsafe readonly byte* data;
		private unsafe readonly ulong size;
#pragma warning restore 0649

		public unsafe object Deserialize() => MsgPackDeserializer.DeserializeAsObject(data, (long)size);
		internal unsafe object[] DeserializeArray() => MsgPackDeserializer.DeserializeAsObjectArray(data, (long)size);
	}
}
