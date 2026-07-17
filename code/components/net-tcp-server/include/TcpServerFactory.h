/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
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