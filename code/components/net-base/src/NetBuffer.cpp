/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "NetBuffer.h"

namespace net
{
Buffer::Buffer()
{
	Initialize();

	m_bytes = std::make_shared<std::vector<uint8_t>>();
}

Buffer::Buffer(const uint8_t* bytes, size_t length)
{
	Initialize();

	m_bytes = std::make_shared<std::vector<uint8_t>>(length);

	memcpy(&(*m_bytes)[0], bytes, m_bytes->size());
}

Buffer::Buffer(const std::vector<uint8_t>& origBytes)
{
	Initialize();

	m_bytes = std::make_shared<std::vector<uint8_t>>(origBytes);
}

Buffer::Buffer(size_t length)
{
	Initialize();

	m_bytes = std::make_shared<std::vector<uint8_t>>(length);
}

Buffer::Buffer(const Buffer& other)
{
	m_curOff = other.m_curOff;
	m_end = other.m_end;

	m_bytes = other.m_bytes;
}

Buffer::Buffer(Buffer&& other)
{
	m_curOff = std::move(other.m_curOff);
	m_end = std::move(other.m_end);

	m_bytes = std::move(other.m_bytes);
}

Buffer& Buffer::operator=(const Buffer& other)
{
	m_curOff = other.m_curOff;
	m_end = other.m_end;

	m_bytes = other.m_bytes;

	return *this;
}

Buffer& Buffer::operator=(Buffer&& other)
{
	m_curOff = std::move(other.m_curOff);
	m_end = std::move(other.m_end);

	m_bytes = std::move(other.m_bytes);

	return *this;
}

Buffer Buffer::Clone() const
{
	Buffer otherBuf(*GetBytes());
	otherBuf.m_end = m_end;
	otherBuf.m_curOff = m_curOff;

	return otherBuf;
}

void Buffer::Initialize()
{
	m_curOff = 0;
	m_end = false;
}

bool Buffer::Read(void* buffer, size_t length)
{
	if (EndsAfterRead(length))
	{
		m_end = true;

		// and if it really doesn't fit out of our buffer
		if (!CanRead(length))
		{
			memset(buffer, 0x00, length);
			return false;
		}
	}

	memcpy(buffer, &(*m_bytes)[m_curOff], length);
	m_curOff += length;

	return true;
}

void Buffer::EnsureWritableSize(size_t length)
{
	if ((m_curOff + length) >= m_bytes->size())
	{
		m_bytes->resize(m_curOff + length);
	}
}

void Buffer::Write(const void* buffer, size_t length)
{
	EnsureWritableSize(length);

	memcpy(&(*m_bytes)[m_curOff], buffer, length);
	m_curOff += length;
}

bool Buffer::ReadTo(Buffer& other, size_t length)
{
	other.EnsureWritableSize(length);

	if ((m_curOff + length) > m_bytes->size())
	{
		return false;
	}

	auto bytes = GetBytes();
	auto otherBytes = other.GetBytes();

	memcpy(&(*otherBytes)[other.GetCurOffset()], &(*bytes)[GetCurOffset()], length);

	m_curOff += length;
	other.m_curOff += length; // ugly :(

	return true;
}

bool Buffer::IsAtEnd() const
{
	return (m_end || m_curOff == m_bytes->size());
}
}
