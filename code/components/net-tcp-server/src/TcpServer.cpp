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
}