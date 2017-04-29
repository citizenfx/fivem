using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security;
using System.Runtime.InteropServices;

namespace CitizenFX.Core
{
    class RemoteFunctionReference : IDisposable
    {
        private readonly string m_reference;

        public RemoteFunctionReference(byte[] reference)
        {
            m_reference = Encoding.UTF8.GetString(reference);
        }

        private bool m_disposed = false;

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (m_disposed)
            {
                return;
            }

            if (disposing)
            {
                // no managed objects to free
            }

            FreeNativeReference();

            m_disposed = true;
        }

        private void FreeNativeReference()
        {
            Native.Function.Call(Native.Hash.DELETE_FUNCTION_REFERENCE, m_reference);
        }

        public byte[] Duplicate()
        {
            return Encoding.UTF8.GetBytes(Native.Function.Call<string>(Native.Hash.DUPLICATE_FUNCTION_REFERENCE, m_reference));
        }

        [SecuritySafeCritical]
        public byte[] InvokeNative(byte[] argsSerialized)
        {
            return _InvokeNative(argsSerialized);
        }

        [SecurityCritical]
        private byte[] _InvokeNative(byte[] argsSerialized)
        {
	        IntPtr resBytes;
            long retLength;

            unsafe
            {
                fixed (byte* argsSerializedRef = &argsSerialized[0])
                {
                    resBytes = Native.Function.Call<IntPtr>(Native.Hash.INVOKE_FUNCTION_REFERENCE, m_reference, argsSerializedRef, argsSerialized.Length, &retLength);
                }
            }

            var retval = new byte[retLength];
            Marshal.Copy(resBytes, retval, 0, retval.Length);

            return retval;
        }
    }
}
