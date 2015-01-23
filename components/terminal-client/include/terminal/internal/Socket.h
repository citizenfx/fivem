/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace terminal
{
class SocketReadResult;

class StreamSocket : public fwRefCountable
{
public:
	virtual concurrency::task<Result<void>> Connect(const char* hostname, uint16_t port) = 0;

	virtual concurrency::task<Result<SocketReadResult>> Read(size_t bytes) = 0;

	virtual concurrency::task<Result<void>> Write(const std::vector<uint8_t>& data) = 0;
};

class SocketReadResult
{
private:
	std::vector<uint8_t> m_buffer;

public:
	inline SocketReadResult()
	{

	}

	inline SocketReadResult(const std::vector<uint8_t>& data)
	{
		m_buffer = data;
	}

	inline const std::vector<uint8_t>& GetBuffer() const
	{
		return m_buffer;
	}
};
}