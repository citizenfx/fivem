/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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
		m_uvLoop->AssertThread();

		return m_uvLoop->GetLoop();
	}
};
}

DECLARE_INSTANCE_TYPE(net::TcpServerManager);
