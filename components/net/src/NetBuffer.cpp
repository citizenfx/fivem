/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

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

bool NetBuffer::Read(void* buffer, size_t length)
{
	if ((m_curOff + length) >= m_length)
	{
		m_end = true;

		// and if it really doesn't fit out of our buffer
		if ((m_curOff + length) > m_length)
		{
			memset(buffer, 0xCE, length);
			return false;
		}
	}

	memcpy(buffer, &m_bytes[m_curOff], length);
	m_curOff += length;

	return true;
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