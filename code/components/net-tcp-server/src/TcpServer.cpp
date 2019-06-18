/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "TcpServer.h"
#include "memdbgon.h"

namespace net
{
TcpServer::TcpServer()
{

}

void TcpServer::SetConnectionCallback(const TConnectionCallback& callback)
{
	m_connectionCallback = callback;
}

void TcpServerStream::Write(const std::string& data)
{
	std::vector<uint8_t> dataBuf(data.size());
	memcpy(dataBuf.data(), data.data(), dataBuf.size());

	Write(std::move(dataBuf));
}

void TcpServerStream::Write(std::string&& data)
{
	Write(static_cast<const std::string&>(data));
}

void TcpServerStream::Write(std::vector<uint8_t>&& data)
{
	Write(static_cast<const std::vector<uint8_t>&>(data));
}

void TcpServerStream::SetCloseCallback(const TCloseCallback& callback)
{
	m_closeCallback = callback;
}

void TcpServerStream::SetReadCallback(const TReadCallback& callback)
{
	bool wasFirst = !static_cast<bool>(m_readCallback);

	m_readCallback = callback;

	if (wasFirst)
	{
		OnFirstSetReadCallback();
	}
}

void TcpServerStream::ScheduleCallback(const TScheduledCallback& callback)
{
	callback();
}
}
