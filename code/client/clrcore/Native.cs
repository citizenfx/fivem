using System;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core.Native
{
    public static class Function
    {
        public static T Call<T>(Hash hash, params InputArgument[] arguments)
        {
			object obj = InvokeInternal(hash, typeof(T), arguments);

			if (PointerArgumentSafety.ShouldClean((ulong)hash, typeof(T)))
			{
				return default;
			}

			return (T)obj;
        }

        public static void Call(Hash hash, params InputArgument[] arguments)
        {
            InvokeInternal(hash, typeof(void), arguments);
        }

		private static unsafe object InvokeInternal(Hash nativeHash, Type returnType, InputArgument[] args)
		{
			ScriptContext.Reset();

			foreach (var arg in args)
			{
				ScriptContext.Push(arg.Value);
			}

#if !IS_FXSERVER
			const int bufferCount = 32;

			// Note: direct access to argument buffer
			fixed (byte* p_functionData = ScriptContext.m_extContext.functionData)
			{
				ulong* argumentBuffer = (ulong*)p_functionData;
				ulong* initialValues = stackalloc ulong[bufferCount];
				// Buffer.MemoryCopy not available on client
				for (uint i = 0; i < bufferCount; ++i)
				{
					initialValues[i] = argumentBuffer[i];
				}
#else
			{
#endif

				ScriptContext.Invoke((ulong)nativeHash, InternalManager.ScriptHost);

				if (returnType == typeof(void))
				{
					return null;
				}

#if !IS_FXSERVER
				if (returnType == typeof(string) && argumentBuffer[0] != 0)
				{
					NativeStringResultSanitization(nativeHash, args, argumentBuffer, ScriptContext.m_extContext.numArguments, initialValues);
				}
#endif
				return ScriptContext.GetResult(returnType);
			}
		}

		/// <summary>
		/// Sanitization for string result types
		/// Loops through all values given by the ScRT and deny any that equals the result value which isn't of the string type
		/// </summary>
		/// <returns>Result from <see cref="ScriptContext.GetResult(Type)"/> or null if sanitized</returns>
		private static unsafe void NativeStringResultSanitization(Hash hash, InputArgument[] inputArguments, ulong* arguments, int numArguments, ulong* initialArguments)
		{
			var resultValue = arguments[0];

			// Step 1: quick compare all values until we found a hit
			// By not switching between all the buffers (incl. input arguments) we'll not introduce unnecessary cache misses.
			for (int a = 0; a < numArguments; ++a)
			{
				if (initialArguments[a] == resultValue)
				{
					// Step 2: loop our input list for as many times as `a` was increased
					int inputSize = inputArguments.Length;
					for (int i = 0; i < inputSize; ++i)
					{
						var csArg = inputArguments[i];

						// `a` can be reused by simply decrementing it, we'll go negative when we hit our goal as we decrement before checking (e.g.: `0 - 1 = -1` or `0 - 4 = -4`)
						switch (csArg?.Value)
						{
							case Vector2 v2:
								a -= 2;
								break;
							case Vector3 v3:
								a -= 3;
								break;
							case Vector4 v4:
							case Quaternion q:
								a -= 4;
								break;
							default:
								a--;
								break;
						}

						// string type is allowed
						if (a < 0)
						{
							if (csArg?.Value?.GetType() != typeof(string))
							{
								Debug.WriteLine($"Warning: Sanitized coerced string result for native {hash}");
								arguments[0] = 0;
							}

							return; // we found our arg, no more to check
						}
					}

					return; // found our value, no more to check
				}
			}
		}
	}

	[StructLayout(LayoutKind.Explicit)]
    internal struct NativeVector3
    {
		[FieldOffset(0)]
        public float X;

		[FieldOffset(8)]
		public float Y;

		[FieldOffset(16)]
		public float Z;

		public static implicit operator NativeVector3(Vector3 v)
		{
			return new NativeVector3() { X = v.X, Y = v.Y, Z = v.Z };
		}

        public static implicit operator Vector3(NativeVector3 self)
        {
            return new Vector3(self.X, self.Y, self.Z);
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

		public static implicit operator InputArgument(Vector3 value)
		{
			return new InputArgument(value);
		}

		public static implicit operator InputArgument(Delegate value)
		{
			return new InputArgument(InternalManager.CanonicalizeRef(FunctionReference.Create(value).Identifier));
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
			return GetResultInternal<T>();
		}

		[SecurityCritical]
		private unsafe T GetResultInternal<T>()
		{
			var data = new byte[24];
			Marshal.Copy(m_dataPtr, data, 0, 24);

			// no native commands include `char**` or `scrObject**` arguments, so these are invalid here
			// see https://github.com/citizenfx/fivem/issues/1855
			//
			// this *might* break struct workarounds but these aren't considered as supported anyway
			if (typeof(T) == typeof(string) || typeof(T) == typeof(object))
			{
				return default(T);
			}

			fixed (byte* dataPtr = data)
			{
				return (T)ScriptContext.GetResult(typeof(T), dataPtr);
			}
		}

		[SecuritySafeCritical]
		private static IntPtr AllocateData()
		{
			return Marshal.AllocHGlobal(24);
		}
	}
}
