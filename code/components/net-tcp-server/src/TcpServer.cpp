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

void TcpServerStream::Write(const std::string& data, TCompleteCallback&& onComplete)
{
	std::vector<uint8_t> dataBuf(data.size());
	memcpy(dataBuf.data(), data.data(), dataBuf.size());

	Write(std::move(dataBuf), std::move(onComplete));
}

void TcpServerStream::Write(std::string&& data, TCompleteCallback&& onComplete)
{
	Write(static_cast<const std::string&>(data), std::move(onComplete));
}

void TcpServerStream::Write(std::vector<uint8_t>&& data, TCompleteCallback&& onComplete)
{
	Write(static_cast<const std::vector<uint8_t>&>(data), std::move(onComplete));
}

void TcpServerStream::Write(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete)
{
	std::vector<uint8_t> dataVec(size);
	memcpy(&dataVec[0], data.get(), size);

	Write(std::move(dataVec), std::move(onComplete));
}

void TcpServerStream::SetCloseCallback(const TCloseCallback& callback)
{
	{
		std::unique_lock<std::shared_mutex> _(m_cbMutex);
		m_closeCallback = callback;
	}
}

void TcpServerStream::SetReadCallback(const TReadCallback& callback)
{
	bool wasFirst = false;

	{
		std::unique_lock<std::shared_mutex> _(m_cbMutex);

		wasFirst = !static_cast<bool>(m_readCallback);
		m_readCallback = callback;
	}

	if (wasFirst)
	{
		OnFirstSetReadCallback();
	}
}

void TcpServerStream::ScheduleCallback(TScheduledCallback&& callback, bool performInline)
{
	callback();
}
}
