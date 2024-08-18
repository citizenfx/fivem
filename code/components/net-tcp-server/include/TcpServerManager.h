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

	uint16_t m_tcpConnectionTimeoutSeconds { 5 };

public:
	TcpServerManager(const std::string& loopName = std::string("default"));

	virtual ~TcpServerManager();

public:
	virtual fwRefContainer<TcpServer> CreateServer(const PeerAddress& bindAddress) override;

	std::shared_ptr<uvw::Loop> GetCurrentWrapLoop();

	uv_loop_t* GetCurrentLoop();

	inline std::shared_ptr<uvw::Loop> GetWrapLoop()
	{
		return m_uvLoop->Get();
	}

	inline uv_loop_t* GetLoop()
	{
		return m_uvLoop->GetLoop();
	}

	uint16_t* GetTcpConnectionTimeoutSeconds()
	{
		return &m_tcpConnectionTimeoutSeconds;
	}

public:
	// callback that can be invoked *from any thread*
	// cancel callback to reject connection
	fwEvent<const net::PeerAddress&> OnStartConnection;

	// callback that can be invoked *from any thread*
	fwEvent<const net::PeerAddress&> OnCloseConnection;
};
}

DECLARE_INSTANCE_TYPE(net::TcpServerManager);
