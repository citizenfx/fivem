/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetAddress.h"

#ifdef COMPILING_NET_TCP_SERVER
#define TCP_SERVER_EXPORT DLL_EXPORT
#else
#define TCP_SERVER_EXPORT DLL_IMPORT
#endif

namespace net
{
class TCP_SERVER_EXPORT TcpServerStream : public fwRefCountable
{
public:
	typedef std::function<void(const std::vector<uint8_t>&)> TReadCallback;

	typedef std::function<void()> TCloseCallback;

	typedef std::function<void()> TScheduledCallback;

private:
	TReadCallback m_readCallback;

	TCloseCallback m_closeCallback;

protected:
	inline const TReadCallback& GetReadCallback()
	{
		return m_readCallback;
	}

	inline const TCloseCallback& GetCloseCallback()
	{
		return m_closeCallback;
	}

	virtual void OnFirstSetReadCallback() {}

public:
	virtual PeerAddress GetPeerAddress() = 0;

	virtual void Write(const std::string& data);

	virtual void Write(std::string&& data);

	virtual void Write(const std::vector<uint8_t>& data) = 0;

	virtual void Write(std::vector<uint8_t>&& data);

	virtual void Close() = 0;

	void SetReadCallback(const TReadCallback& callback);

	void SetCloseCallback(const TCloseCallback& callback);

	virtual void ScheduleCallback(const TScheduledCallback& callback);

protected:
	TcpServerStream() { }
};

class TCP_SERVER_EXPORT TcpServer : public fwRefCountable
{
public:
	typedef std::function<void(fwRefContainer<TcpServerStream>)> TConnectionCallback;

private:
	TConnectionCallback m_connectionCallback;

public:
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
