/*
 * This file is part of FiveM (https://fivem.net), created by CitizenFX (https://github.com/citizenfx)
 *
 * See root directory for information regarding LICENSE and other instruction.
 * 
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