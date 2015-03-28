/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetAddress.h"

namespace net
{
class TcpServerStream : public fwRefCountable
{
public:
	typedef std::function<void(const std::vector<uint8_t>&)> TReadCallback;

private:
	TReadCallback m_readCallback;

protected:
	inline const TReadCallback& GetReadCallback()
	{
		return m_readCallback;
	}

public:
	virtual PeerAddress GetPeerAddress() = 0;

	void SetReadCallback(const TReadCallback& callback);

protected:
	TcpServerStream() { }
};

class TcpServer : public fwRefCountable
{
public:
	typedef std::function<void(fwRefContainer<TcpServerStream>)> TConnectionCallback;

private:
	TConnectionCallback m_connectionCallback;

protected:
	inline const TConnectionCallback& GetConnectionCallback()
	{
		return m_connectionCallback;
	}

public:
	void SetConnectionCallback(const TConnectionCallback& callback);

protected:
	// don't allow construction
	TcpServer();
};
}