// SharedMemory (File: SharedMemory\FastStructure.cs)
// Copyright (c) 2014 Justin Stenning
// http://spazzarama.com
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
// The SharedMemory library is inspired by the following Code Project article:
//   "Fast IPC Communication Using Shared Memory and InterlockedCompareExchange"
//   http://www.codeproject.com/Articles/14740/Fast-IPC-Communication-Using-Shared-Memory-and-Int
using System;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Security;

namespace CitizenFX.Core
{
    /// <summary>
    /// Provides fast reading and writing of generic structures to a memory location using IL emitted functions.
    /// </summary>
    internal static class FastStructure
    {
        /// <summary>
        /// Retrieve a pointer to the passed generic structure type. This is achieved by emitting a <see cref="DynamicMethod"/> to retrieve a pointer to the structure.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="structure"></param>
        /// <returns>A pointer to the provided structure in memory.</returns>
        /// <see cref="FastStructure{T}.GetPtr"/>
		[SecurityCritical]
        public static unsafe void* GetPtr<T>(ref T structure)
            where T : struct
        {
            return FastStructure<T>.GetPtr(ref structure);
        }

		/// <summary>
		/// Loads the generic value type <typeparamref name="T"/> from a pointer. This is achieved by emitting a <see cref="DynamicMethod"/> that returns the value in the memory location as a <typeparamref name="T"/>.
		/// <para>The equivalent non-generic C# code:</para>
		/// <code>
		/// unsafe MyStruct ReadFromPointer(byte* pointer)
		/// {
		///     return *(MyStruct*)pointer;
		/// }
		/// </code>
		/// </summary>
		/// <typeparam name="T">Any value/structure type</typeparam>
		/// <param name="pointer">Unsafe pointer to memory to load the value from</param>
		/// <returns>The newly loaded value</returns>
		[SecurityCritical]
		public static unsafe T PtrToStructure<T>(IntPtr pointer)
            where T : struct
        {
            return FastStructure<T>.PtrToStructure(pointer);
        }

		/// <summary>
		/// Writes the generic value type <typeparamref name="T"/> to the location specified by a pointer. This is achieved by emitting a <see cref="DynamicMethod"/> that copies the value from the referenced structure into the specified memory location.
		/// <para>There is no exact equivalent possible in C#, the closest possible (generates the same IL) is the following code:</para>
		/// <code>
		/// unsafe void WriteToPointer(ref SharedHeader dest, ref SharedHeader src)
		/// {
		///     dest = src;
		/// }
		/// </code>
		/// </summary>
		/// <typeparam name="T"></typeparam>
		/// <param name="pointer"></param>
		/// <param name="structure"></param>
		[SecurityCritical]
		public static unsafe void StructureToPtr<T>(ref T structure, IntPtr pointer)
            where T : struct
        {
            FastStructure<T>.StructureToPtr(ref structure, pointer);
        }

        /// <summary>
        /// Retrieve the cached size of a structure
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        /// <remarks>Caches the size by type</remarks>
        /// <see cref="FastStructure{T}.Size"/>
        public static int SizeOf<T>()
            where T : struct
        {
            return FastStructure<T>.Size;
        }

		/// <summary>
		/// Reads a number of elements from a memory location into the provided buffer starting at the specified index.
		/// </summary>
		/// <typeparam name="T">The structure type</typeparam>
		/// <param name="buffer">The destination buffer.</param>
		/// <param name="source">The source memory location.</param>
		/// <param name="index">The start index within <paramref name="buffer"/>.</param>
		/// <param name="count">The number of elements to read.</param>
		[SecurityCritical]
		public static unsafe void ReadArray<T>(T[] buffer, IntPtr source, int index, int count)
            where T : struct
        {
            uint elementSize = (uint)SizeOf<T>();

            if (buffer == null)
                throw new ArgumentNullException("buffer");
            if (count < 0)
                throw new ArgumentOutOfRangeException("count");
            if (index < 0)
                throw new ArgumentOutOfRangeException("index");
            if (buffer.Length - index < count)
                throw new ArgumentException("Invalid offset into array specified by index and count");

            void* ptr = source.ToPointer();
            byte* p = (byte*)FastStructure.GetPtr<T>(ref buffer[0]);
            Buffer.MemoryCopy(ptr, p + (index * elementSize), elementSize * count, elementSize * count);
        }

		/// <summary>
		/// Writes a number of elements to a memory location from the provided buffer starting at the specified index.
		/// </summary>
		/// <typeparam name="T">The structure type</typeparam>
		/// <param name="destination">The destination memory location.</param>
		/// <param name="buffer">The source buffer.</param>
		/// <param name="index">The start index within <paramref name="buffer"/>.</param>
		/// <param name="count">The number of elements to write.</param>
		[SecurityCritical]
		public static unsafe void WriteArray<T>(IntPtr destination, T[] buffer, int index, int count)
            where T : struct
        {
            uint elementSize = (uint)SizeOf<T>();

            if (buffer == null)
                throw new ArgumentNullException("buffer");
            if (count < 0)
                throw new ArgumentOutOfRangeException("count");
            if (index < 0)
                throw new ArgumentOutOfRangeException("index");
            if (buffer.Length - index < count)
                throw new ArgumentException("Invalid offset into array specified by index and count");

            void* ptr = destination.ToPointer();
            byte* p = (byte*)FastStructure.GetPtr<T>(ref buffer[0]);
            Buffer.MemoryCopy(p + (index * elementSize), ptr, elementSize * count, elementSize * count);
        }
    }

    /// <summary>
    /// Emits optimized IL for the reading and writing of structures to/from memory.
    /// <para>For a 32-byte structure with 1 million iterations:</para>
    /// <para>The <see cref="FastStructure{T}.PtrToStructure"/> method performs approx. 20x faster than
    /// <see cref="System.Runtime.InteropServices.Marshal.PtrToStructure(IntPtr, Type)"/> (8ms vs 160ms), and about 1.6x slower than the non-generic equivalent (8ms vs 5ms)</para>
    /// <para>The <see cref="FastStructure{T}.StructureToPtr"/> method performs approx. 8x faster than 
    /// <see cref="System.Runtime.InteropServices.Marshal.StructureToPtr(object, IntPtr, bool)"/> (4ms vs 34ms). </para>
    /// </summary>
    /// <typeparam name="T"></typeparam>
    public static class FastStructure<T>
        where T : struct
    {
        /// <summary>
        /// Delegate that returns a pointer to the provided structure. Use with extreme caution.
        /// </summary>
        /// <param name="value"></param>
        /// <returns></returns>
        public unsafe delegate void* GetPtrDelegate(ref T value);
        
        /// <summary>
        /// Delegate for loading a structure from the specified memory address
        /// </summary>
        /// <param name="pointer"></param>
        /// <returns></returns>
        public delegate T PtrToStructureDelegate(IntPtr pointer);
        
        /// <summary>
        /// Delegate for writing a structure to the specified memory address
        /// </summary>
        /// <param name="value"></param>
        /// <param name="pointer"></param>
        public delegate void StructureToPtrDelegate(ref T value, IntPtr pointer);
        
        /// <summary>
        /// The <see cref="GetPtrDelegate"/> delegate for the generated IL to retrieve a pointer to the structure
        /// </summary>
        public unsafe readonly static GetPtrDelegate GetPtr = BuildFunction();

        /// <summary>
        /// The <see cref="PtrToStructureDelegate"/> delegate for the generated IL to retrieve a structure from a specified memory address.
        /// </summary>
        public readonly static PtrToStructureDelegate PtrToStructure = BuildLoadFromPointerFunction();

        /// <summary>
        /// The <see cref="StructureToPtrDelegate"/> delegate for the generated IL to store a structure at the specified memory address.
        /// </summary>
        public readonly static StructureToPtrDelegate StructureToPtr = BuildWriteToPointerFunction();

        /// <summary>
        /// Cached size of T as determined by <see cref="System.Runtime.InteropServices.Marshal.SizeOf(Type)"/>.
        /// </summary>
        public static readonly int Size = System.Runtime.InteropServices.Marshal.SizeOf(typeof(T));
        
        private static DynamicMethod method;
        private static DynamicMethod methodLoad;
        private static DynamicMethod methodWrite;

		/// <summary>
		/// Performs once of type compatibility check.
		/// </summary>
		/// <exception cref="ArgumentException">Thrown if the type T is incompatible</exception>
		[SecuritySafeCritical]
		static FastStructure()
        {
            // Performs compatibility checks upon T
            CheckTypeCompatibility(typeof(T));
        }

		[SecurityCritical]
		private unsafe static GetPtrDelegate BuildFunction()
        {
            method = new DynamicMethod("GetStructurePtr<" + typeof(T).FullName + ">",
                typeof(void*), new Type[1] { typeof(T).MakeByRefType() }, typeof(FastStructure).Module);

            ILGenerator generator = method.GetILGenerator();
            generator.Emit(OpCodes.Ldarg_0);
            generator.Emit(OpCodes.Conv_U);
            generator.Emit(OpCodes.Ret);
            return (GetPtrDelegate)method.CreateDelegate(typeof(GetPtrDelegate));
        }

		[SecurityCritical]
		private static unsafe PtrToStructureDelegate BuildLoadFromPointerFunction()
        {
            methodLoad = new DynamicMethod("PtrToStructure<" + typeof(T).FullName + ">",
                typeof(T), new Type[1] { typeof(IntPtr) }, typeof(FastStructure).Module);

            ILGenerator generator = methodLoad.GetILGenerator();
            generator.Emit(OpCodes.Ldarg_0);
            generator.Emit(OpCodes.Ldobj, typeof(T));
            generator.Emit(OpCodes.Ret);

            return (PtrToStructureDelegate)methodLoad.CreateDelegate(typeof(PtrToStructureDelegate));
        }

		[SecurityCritical]
		private static unsafe StructureToPtrDelegate BuildWriteToPointerFunction()
        {
            methodWrite = new DynamicMethod("StructureToPtr<" + typeof(T).FullName + ">",
                null, new Type[2] { typeof(T).MakeByRefType(), typeof(IntPtr) }, typeof(FastStructure).Module);

            ILGenerator generator = methodWrite.GetILGenerator();
            generator.Emit(OpCodes.Ldarg_1);
            generator.Emit(OpCodes.Ldarg_0);
            generator.Emit(OpCodes.Ldobj, typeof(T));
            generator.Emit(OpCodes.Stobj, typeof(T));
            generator.Emit(OpCodes.Ret);
            return (StructureToPtrDelegate)methodWrite.CreateDelegate(typeof(StructureToPtrDelegate));
        }

		[SecuritySafeCritical]
		private static void CheckTypeCompatibility(Type t, System.Collections.Generic.HashSet<Type> checkedItems = null)
        {
            if (checkedItems == null)
            {
                checkedItems = new System.Collections.Generic.HashSet<Type>();
                checkedItems.Add(typeof(char));
                checkedItems.Add(typeof(byte));
                checkedItems.Add(typeof(sbyte));
                checkedItems.Add(typeof(bool));
                checkedItems.Add(typeof(double));
                checkedItems.Add(typeof(float));
                checkedItems.Add(typeof(decimal));
                checkedItems.Add(typeof(int));
                checkedItems.Add(typeof(short));
                checkedItems.Add(typeof(long));
                checkedItems.Add(typeof(uint));
                checkedItems.Add(typeof(ushort));
                checkedItems.Add(typeof(ulong));
                checkedItems.Add(typeof(IntPtr));
                checkedItems.Add(typeof(void*));
            }

            if (checkedItems.Contains(t))
                return;
            else
                checkedItems.Add(t);

            FieldInfo[] fi = t.GetFields(BindingFlags.Public | BindingFlags.Instance | BindingFlags.NonPublic);
            foreach (FieldInfo info in fi)
            {
                if (!info.FieldType.IsPrimitive && !info.FieldType.IsValueType && !info.FieldType.IsPointer)
                {
                    throw new ArgumentException(String.Format("Non-value types are not supported: field {0} is of type {1} in structure {2}", info.Name, info.FieldType.Name, info.DeclaringType.Name));
                }

                // Example for adding future marshal attributes as incompatible
                //System.Runtime.InteropServices.MarshalAsAttribute attr;
                //if (TryGetAttribute<System.Runtime.InteropServices.MarshalAsAttribute>(info, out attr))
                //{
                //    if (attr.Value == System.Runtime.InteropServices.UnmanagedType.ByValArray)
                //    {
                //        throw new ArgumentException(String.Format("UnmanagedType.ByValArray is not supported on field {0} in type [{1}].", info.Name, typeof(T).FullName));
                //    }
                //}

                CheckTypeCompatibility(info.FieldType, checkedItems);
            }
        }

        //private static bool TryGetAttribute<T1>(MemberInfo memberInfo, out T1 customAttribute) where T1 : Attribute
        //{
        //    var attributes = memberInfo.GetCustomAttributes(typeof(T1), false).FirstOrDefault();
        //    if (attributes == null)
        //    {
        //        customAttribute = null;
        //        return false;
        //    }
        //    customAttribute = (T1)attributes;
        //    return true;
        //}
    }
}
