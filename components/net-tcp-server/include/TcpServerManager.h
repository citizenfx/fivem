/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetAddress.h"
#include "UvLoopHolder.h"

#include "UvTcpServer.h"
#include "TcpServerFactory.h"

#include <memory>

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class TCP_SERVER_EXPORT TcpServerManager : public TcpServerFactory
{
private:
	std::set<fwRefContainer<UvTcpServer>> m_servers;

	fwRefContainer<UvLoopHolder> m_uvLoop;

public:
	TcpServerManager();

	virtual ~TcpServerManager();

public:
	virtual fwRefContainer<TcpServer> CreateServer(const PeerAddress& bindAddress) override;

	inline uv_loop_t* GetLoop()
	{
		return m_uvLoop->GetLoop();
	}
};
}