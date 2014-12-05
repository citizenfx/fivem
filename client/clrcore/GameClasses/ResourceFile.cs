using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Security;
using System.Text;
using System.Threading.Tasks;

namespace CitizenFX.Core
{
    public static class ResourceFile
    {
        public static Stream OpenRead(string fileName)
        {
            var fileHandle = Function.Call<int>(Natives.OPEN_FILE_FOR_READING, fileName);

            if (fileHandle == -1)
            {
                throw new IOException("Opening file " + fileName + " for reading failed.");
            }

            return new ResourceFileStream(fileHandle);
        }

        private class ResourceFileStream : Stream
        {
            private int m_handle;

            public ResourceFileStream(int handle)
            {
                m_handle = handle;
            }

            public override long Seek(long offset, SeekOrigin origin)
            {
                throw new NotImplementedException();
            }

            public override bool CanRead
            {
                get { return true; }
            }

            public override bool CanSeek
            {
                get { return false; }
            }

            public override bool CanWrite
            {
                get { return false; }
            }

            public override long Length
            {
                get { return Function.Call<int>(Natives.GET_LENGTH_OF_FILE, m_handle); }
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
            public unsafe override int Read(byte[] buffer, int offset, int count)
            {
                Pointer lengthPtr = typeof(int);
                int inBufferPtr = Function.Call<int>(Natives.READ_FILE, m_handle, count, lengthPtr);

                IntPtr inBuffer = new IntPtr(*(uint*)&inBufferPtr);
                Marshal.Copy(inBuffer, buffer, offset, count);

                Function.Call(Natives.FREE_FILE_BUFFER, inBufferPtr);

                return (int)lengthPtr;
            }

            public override void Write(byte[] buffer, int offset, int count)
            {
                throw new NotImplementedException();
            }

            public override void SetLength(long value)
            {
                throw new NotImplementedException();
            }

            public override void Flush()
            {
                
            }

            public override void Close()
            {
                Function.Call(Natives.CLOSE_FILE, m_handle);
            }
        }
    }
}
