/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include "NetAddress.h"
#include <function2.hpp>

#include <shared_mutex>

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

	typedef fu2::unique_function<void()> TScheduledCallback;

	typedef fu2::unique_function<void(bool)> TCompleteCallback;

private:
	TReadCallback m_readCallback;

	TCloseCallback m_closeCallback;

	std::shared_mutex m_cbMutex;

protected:
	inline TReadCallback GetReadCallback()
	{
		std::shared_lock<std::shared_mutex> _(m_cbMutex);
		return m_readCallback;
	}

	inline TCloseCallback GetCloseCallback()
	{
		std::shared_lock<std::shared_mutex> _(m_cbMutex);
		return m_closeCallback;
	}

	virtual void OnFirstSetReadCallback() {}

public:
	virtual PeerAddress GetPeerAddress() = 0;

	virtual void Write(const std::string& data, TCompleteCallback&& onComplete = {});

	virtual void Write(std::string&& data, TCompleteCallback&& onComplete = {});

	virtual void Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete = {}) = 0;

	virtual void Write(std::vector<uint8_t>&& data, TCompleteCallback&& onComplete = {});

	virtual void Write(std::unique_ptr<char[]> data, size_t size, TCompleteCallback&& onComplete = {});

	virtual void Close() = 0;

	void SetReadCallback(const TReadCallback& callback);

	void SetCloseCallback(const TCloseCallback& callback);

	virtual void ScheduleCallback(TScheduledCallback&& callback, bool performInline = true);

	virtual void StartConnectionTimeout(std::chrono::duration<uint64_t, std::milli> timeout) = 0;
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
