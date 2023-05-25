using System;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core.Native
{
	internal static class MemoryAccess
	{
		public static int[] GetPickupObjectHandles() => new int[0];
		public static int[] GetPedHandles() => new int[0];
		public static int[] GetEntityHandles() => new int[0];
		public static int[] GetPropHandles() => new int[0];
		public static int[] GetVehicleHandles() => new int[0];

		public static int[][] VehicleModels => null;

		public static float ReadWorldGravity()
		{
			return 0.0f;
		}

		public static void WriteWorldGravity(float f)
		{

		}

		[SecurityCritical]
		public static unsafe T GetValueOrDefault<T>(IntPtr ptr, int offset, T def) where T : unmanaged => ptr != IntPtr.Zero ? *(T*)(ptr + offset) : def;
		[SecurityCritical]
		public static unsafe void SetValueIfNotNull<T>(IntPtr ptr, int offset, T val) where T : unmanaged
		{
			if (ptr != IntPtr.Zero)
				*(T*)(ptr + offset) = val;
		}

		[SecuritySafeCritical] public static byte ReadByte(IntPtr pointer) => Marshal.ReadByte(pointer);
		[SecuritySafeCritical] public static short ReadShort(IntPtr pointer) => Marshal.ReadInt16(pointer);
		[SecuritySafeCritical] public static int ReadInt(IntPtr pointer) => Marshal.ReadInt32(pointer);
		[SecuritySafeCritical] public static IntPtr ReadPtr(IntPtr pointer) => Marshal.ReadIntPtr(pointer);
		[SecuritySafeCritical] public static float ReadFloat(IntPtr pointer) => BitConverter.ToSingle(BitConverter.GetBytes(Marshal.ReadInt32(pointer)), 0);
		[SecuritySafeCritical] public static Matrix ReadMatrix(IntPtr pointer) => Marshal.PtrToStructure<Matrix>(pointer);
		[SecuritySafeCritical] public static Vector3 ReadVector3(IntPtr pointer) => Marshal.PtrToStructure<Vector3>(pointer);

		[SecuritySafeCritical] public static void WriteByte(IntPtr pointer, byte value) => Marshal.WriteByte(pointer, value);
		[SecuritySafeCritical] public static void WriteShort(IntPtr pointer, short value) => Marshal.WriteInt16(pointer, value);
		[SecuritySafeCritical] public static void WriteInt(IntPtr pointer, int value) => Marshal.WriteInt32(pointer, value);
		[SecuritySafeCritical] public static void WriteFloat(IntPtr pointer, float value) => Marshal.WriteInt32(pointer, BitConverter.ToInt32(BitConverter.GetBytes(value), 0));
		[SecuritySafeCritical] public static void WriteVector3(IntPtr pointer, Vector3 value) => Marshal.StructureToPtr(value, pointer, false);

		[SecuritySafeCritical] public static unsafe bool IsBitSet(IntPtr pointer, int bit) => (*(int*)pointer & (1 << bit)) != 0;
		[SecuritySafeCritical] public static unsafe void ClearBit(IntPtr pointer, int bit) => *(int*)pointer &= ~(1 << bit);
		[SecuritySafeCritical] public static unsafe void SetBit(IntPtr pointer, int bit) => *(int*)pointer |= 1 << bit;

		public static IntPtr StringPtr { get; } = Marshal.StringToHGlobalAnsi("STRING");
		public static IntPtr NullString { get; } = Marshal.StringToHGlobalAnsi("");
		public static IntPtr CellEmailBcon { get; } = Marshal.StringToHGlobalAnsi("CELL_EMAIL_BCON");
	}
}
