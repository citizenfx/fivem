using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Security;

namespace CitizenFX.Core
{
    class RemoteFunctionReference : IDisposable
    {
        private string m_resource;
        private uint m_instance;
        private uint m_reference;

        public RemoteFunctionReference(string resource, uint instance, uint reference)
        {
            m_resource = resource;
            m_instance = instance;
            m_reference = reference;
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

        [SecuritySafeCritical]
        private void FreeNativeReference()
        {
            GameInterface.DeleteNativeReference(m_resource, m_instance, m_reference);
        }

        [SecuritySafeCritical]
        public byte[] InvokeNative(byte[] argsSerialized)
        {
            return GameInterface.InvokeNativeReference(m_resource, m_instance, m_reference, argsSerialized);
        }
    }
}
