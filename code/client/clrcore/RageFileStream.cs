using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security;
using System.Text;
using System.Threading.Tasks;

#if nope
namespace CitizenFX.Core
{
    public class RageFileStream : Stream
    {
        private IntPtr m_handle;

        [SecuritySafeCritical]
        internal RageFileStream(string fileName)
        {
            // attempt opening the file
            m_handle = GameInterface.OpenFile(fileName);

            if (m_handle == IntPtr.Zero)
            {
                throw new IOException("Could not open RAGE file " + fileName + " for reading.");
            }
        }

        public override bool CanRead
        {
            get { return true; }
        }

        public override bool CanWrite
        {
            get { return false; }
        }

        public override bool CanSeek
        {
            get { return false; }
        }

        [SecuritySafeCritical]
        public override int Read(byte[] buffer, int offset, int count)
        {
            return GameInterface.ReadFile(m_handle, buffer, offset, count);
        }

        public override void Write(byte[] buffer, int offset, int count)
        {
            throw new NotImplementedException();
        }

        public override void SetLength(long value)
        {
            throw new NotImplementedException();
        }

        public override long Seek(long offset, SeekOrigin origin)
        {
            throw new NotImplementedException();
        }

        public override void Flush()
        {
            throw new NotImplementedException();
        }

        public override long Position
        {
            get
            {
                throw new NotImplementedException();
            }
            set
            {
                throw new NotImplementedException();
            }
        }

        [SecuritySafeCritical]
        public override void Close()
        {
            GameInterface.CloseFile(m_handle);

            base.Close();
        }

        public override long Length
        {
            [SecuritySafeCritical]
            get
            {
                return GameInterface.GetFileLength(m_handle);
            }
        }
    }
}
#endif