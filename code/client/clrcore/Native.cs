using System;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core.Native
{
    public static class Function
    {
        public static T Call<T>(Hash hash, params InputArgument[] arguments)
        {
            return (T)InvokeInternal(hash, typeof(T), arguments);
        }

        public static void Call(Hash hash, params InputArgument[] arguments)
        {
            InvokeInternal(hash, typeof(void), arguments);
        }

        private static object InvokeInternal(Hash nativeHash, Type returnType, InputArgument[] args)
        {
            using (var scriptContext = new ScriptContext())
            {
                foreach (var arg in args)
                {
                    scriptContext.Push(arg.Value);
                }

                scriptContext.Invoke((ulong)nativeHash, InternalManager.ScriptHost);

                if (returnType != typeof(void))
                {
                    return scriptContext.GetResult(returnType);
                }

                return null;
            }
        }
    }

    internal struct NativeVector3
    {
        public float X;
        public float Y;
        public float Z;

        public static implicit operator Vector3(NativeVector3 self)
        {
            return Vector3.Zero;
        }
    }

    internal static class MemoryAccess
    {
        public static uint GetHashKey(string input)
        {
            uint hash = 0;
            var len = input.Length;

            input = input.ToLowerInvariant();

            for (var i = 0; i < len; i++)
            {
                hash += input[i];
                hash += (hash << 10);
                hash ^= (hash >> 6);
            }

            hash += (hash << 3);
            hash ^= (hash >> 11);
            hash += (hash << 15);

            return hash;
        }

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

        [SecuritySafeCritical]
        public static byte ReadByte(IntPtr pointer)
        {
            return Marshal.ReadByte(pointer);
        }

        [SecuritySafeCritical]
        public static short ReadShort(IntPtr pointer)
        {
            return Marshal.ReadInt16(pointer);
        }

        [SecuritySafeCritical]
        public static int ReadInt(IntPtr pointer)
        {
            return Marshal.ReadInt32(pointer);
        }

        [SecuritySafeCritical]
        public static IntPtr ReadPtr(IntPtr pointer)
        {
            return Marshal.ReadIntPtr(pointer);
        }

        [SecuritySafeCritical]
        public static void WriteByte(IntPtr pointer, byte value)
        {
            Marshal.WriteByte(pointer, value);
        }

        [SecuritySafeCritical]
        public static void WriteShort(IntPtr pointer, short value)
        {
            Marshal.WriteInt16(pointer, value);
        }

        [SecuritySafeCritical]
        public static void WriteInt(IntPtr pointer, int value)
        {
            Marshal.WriteInt32(pointer, value);
        }

        [SecuritySafeCritical]
        public static void WriteFloat(IntPtr pointer, float value)
        {
            Marshal.WriteInt32(pointer, BitConverter.ToInt32(BitConverter.GetBytes(value), 0));
        }


        [SecuritySafeCritical]
        public static float ReadFloat(IntPtr pointer)
        {
            return BitConverter.ToSingle(BitConverter.GetBytes(Marshal.ReadInt32(pointer)), 0);
        }

        [SecuritySafeCritical]
        public static Matrix ReadMatrix(IntPtr pointer)
        {
            return Marshal.PtrToStructure<Matrix>(pointer);
        }

        [SecuritySafeCritical]
        public static Vector3 ReadVector3(IntPtr pointer)
        {
            return Marshal.PtrToStructure<Vector3>(pointer);
        }

        [SecuritySafeCritical]
        public static void WriteVector3(IntPtr pointer, Vector3 value)
        {
            Marshal.StructureToPtr(value, pointer, false);
        }

        [SecuritySafeCritical]
        public static bool IsBitSet(IntPtr pointer, int bit)
        {
            return _IsBitSet(pointer, bit);
        }

        [SecurityCritical]
        private static bool _IsBitSet(IntPtr pointer, int bit)
        {
            unsafe
            {
                var ptr = (int*)pointer.ToPointer();
                return (*ptr & (1 << bit)) != 0;
            }
        }

        [SecuritySafeCritical]
        public static void ClearBit(IntPtr pointer, int bit)
        {
            _ClearBit(pointer, bit);
        }

        [SecurityCritical]
        private static void _ClearBit(IntPtr pointer, int bit)
        {
            unsafe
            {
                var ptr = (int*)pointer.ToPointer();
                *ptr &= ~(1 << bit);
            }
        }

        [SecuritySafeCritical]
        public static void SetBit(IntPtr pointer, int bit)
        {
            _SetBit(pointer, bit);
        }

        [SecurityCritical]
        private static void _SetBit(IntPtr pointer, int bit)
        {
            unsafe
            {
                int* ptr = (int*)pointer.ToPointer();
                *ptr |= 1 << bit;
            }
        }

        private static IntPtr ms_stringString;

        public static IntPtr StringPtr
        {
            [SecuritySafeCritical]
            get
            {
                if (ms_stringString == IntPtr.Zero)
                {
                    ms_stringString = Marshal.StringToHGlobalAnsi("STRING");
                }

                return ms_stringString;
            }
        }

        private static IntPtr ms_nullString;

        public static IntPtr NullString
        {
            [SecuritySafeCritical]
            get
            {
                if (ms_nullString == IntPtr.Zero)
                {
                    ms_nullString = Marshal.StringToHGlobalAnsi("");
                }

                return ms_nullString;
            }
        }

		private static IntPtr ms_cellEmailBconString;

		public static IntPtr CellEmailBcon
		{
			[SecuritySafeCritical]
			get
			{
				if (ms_cellEmailBconString == IntPtr.Zero)
				{
					ms_cellEmailBconString = Marshal.StringToHGlobalAnsi("CELL_EMAIL_BCON");
				}

				return ms_cellEmailBconString;
			}
		}
    }

    public abstract class INativeValue
    {
        public abstract ulong NativeValue
        {
            get;
            set;
        }
    }

    public class InputArgument
    {
        protected object m_value;

        internal object Value => m_value;

        internal InputArgument(object value)
        {
            m_value = value;
        }

        public override string ToString()
        {
            return m_value.ToString();
        }

        public static implicit operator InputArgument(bool value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(sbyte value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(byte value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(short value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(ushort value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(int value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(uint value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(long value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(ulong value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(float value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(double value)
        {
            return new InputArgument((float)value);
        }

        public static implicit operator InputArgument(Enum value)
        {
            return new InputArgument(value);
        }

        public static implicit operator InputArgument(string value)
        {
            return new InputArgument(value);
        }

        [SecuritySafeCritical]
        public static implicit operator InputArgument(INativeValue value)
        {
            return new InputArgument(value.NativeValue);
        }

        [SecurityCritical]
        public static implicit operator InputArgument(IntPtr value)
        {
            return new InputArgument(value);
        }

        [SecurityCritical]
        public static unsafe implicit operator InputArgument(void* value)
        {
            return new InputArgument(new IntPtr(value));
        }
    }

	public class OutputArgument : InputArgument
	{
		private readonly IntPtr m_dataPtr;

		[SecuritySafeCritical]
		public OutputArgument()
			: base(AllocateData())
		{
			m_dataPtr = (IntPtr)m_value;
		}

		[SecuritySafeCritical]
		public OutputArgument(object arg)
			: this()
		{
			if (Marshal.SizeOf(arg.GetType()) > 8)
			{
				return;
			}

			Marshal.WriteInt64(m_dataPtr, 0, 0);
			Marshal.StructureToPtr(arg, m_dataPtr, false);
		}

		[SecuritySafeCritical]
		~OutputArgument()
		{
			Marshal.FreeHGlobal(m_dataPtr);
		}

		[SecuritySafeCritical]
		public T GetResult<T>()
		{
			var data = new byte[24];
			Marshal.Copy(m_dataPtr, data, 0, 24);

			return (T)ScriptContext.GetResult(typeof(T), data);
		}

		[SecuritySafeCritical]
		private static IntPtr AllocateData()
		{
			return Marshal.AllocHGlobal(24);
		}
	}
}
