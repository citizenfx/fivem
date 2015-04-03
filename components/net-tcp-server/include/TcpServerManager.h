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

namespace net
{
class TcpServerManager : public UvLoopHolder, public TcpServerFactory
{
private:
	std::set<fwRefContainer<UvTcpServer>> m_servers;

public:
	TcpServerManager();

	virtual ~TcpServerManager();

public:
	virtual fwRefContainer<TcpServer> CreateServer(const PeerAddress& bindAddress) override;
};
}