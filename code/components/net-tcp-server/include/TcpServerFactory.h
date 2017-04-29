/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

namespace net
{
class TcpServer;
class PeerAddress;

class TcpServerFactory : public fwRefCountable
{
public:
	virtual fwRefContainer<TcpServer> CreateServer(const PeerAddress& bindAddress) = 0;
};
}