using System;
using System.IO;

namespace CitizenFX.Core
{
	public class FxStreamWrapper : Stream
	{
		private readonly fxIStream m_stream;

		public FxStreamWrapper(fxIStream stream)
		{
			m_stream = stream;
		}

		public override void Flush()
		{
            
		}

		public override long Seek(long offset, SeekOrigin origin)
		{
			return m_stream.Seek(offset, (int)origin);
		}

		public override void SetLength(long value)
		{
            
		}

		public override int Read(byte[] buffer, int offset, int count)
		{
			var inRead = new byte[count];
			var numRead = m_stream.Read(inRead, count);

			Array.Copy(inRead, 0, buffer, offset, count);

			return numRead;
		}

		public override void Write(byte[] buffer, int offset, int count)
		{
			var inWrite = new byte[count];
			Array.Copy(buffer, offset, inWrite, 0, count);

			m_stream.Write(inWrite, count);
		}

		public override bool CanRead => true;

		public override bool CanSeek => true;

		public override bool CanWrite => true;

		public override long Length => m_stream.GetLength();

		public override long Position {
			get { return m_stream.Seek(0, 1); }
			set { m_stream.Seek(value, 0); }
		}
	}
}