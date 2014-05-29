#include "StdInc.h"
#include "NetBuffer.h"

NetBuffer::NetBuffer(const char* bytes, size_t length)
	: m_bytesManaged(false), m_bytes(const_cast<char*>(bytes)), m_curOff(0), m_length(length), m_end(false)
{

}

NetBuffer::NetBuffer(size_t length)
	: m_length(length), m_curOff(0), m_bytesManaged(true), m_end(false)
{
	m_bytes = new char[length];
}

NetBuffer::~NetBuffer()
{
	if (m_bytesManaged)
	{
		delete[] m_bytes;
	}
}

void NetBuffer::Read(void* buffer, size_t length)
{
	if ((m_curOff + length) >= m_length)
	{
		m_end = true;

		// and if it really doesn't fit out of our buffer
		if ((m_curOff + length) > m_length)
		{
			__asm int 3

			memset(buffer, 0xCE, sizeof(length));
			return;
		}
	}

	memcpy(buffer, &m_bytes[m_curOff], length);
	m_curOff += length;
}

void NetBuffer::Write(const void* buffer, size_t length)
{
	if ((m_curOff + length) >= m_length)
	{
		m_end = true;

		if ((m_curOff + length) > m_length)
		{
			return;
		}
	}

	memcpy(&m_bytes[m_curOff], buffer, length);
	m_curOff += length;
}

bool NetBuffer::End()
{
	return (m_end || m_curOff == m_length);
}