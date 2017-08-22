/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <uv.h>

#include <memory>

#include "TcpServer.h"

#include <tbb/concurrent_queue.h>

namespace net
{
class UvTcpServer;

class UvTcpServerStream : public TcpServerStream
{
private:
	UvTcpServer* m_server;

	std::unique_ptr<uv_tcp_t> m_client;

	std::unique_ptr<uv_async_t> m_writeCallback;

	tbb::concurrent_queue<std::function<void()>> m_pendingRequests;

	std::vector<char> m_readBuffer;

private:
	void HandleRead(ssize_t nread, const uv_buf_t* buf);

	void HandlePendingWrites();

	void CloseClient();

	inline std::vector<char>& GetReadBuffer()
	{
		return m_readBuffer;
	}

public:
	UvTcpServerStream(UvTcpServer* server);

	virtual ~UvTcpServerStream();

	bool Accept(std::unique_ptr<uv_tcp_t>&& client);

	virtual void AddRef() override
	{
		TcpServerStream::AddRef();
	}

	virtual bool Release() override
	{
		return TcpServerStream::Release();
	}

public:
	virtual PeerAddress GetPeerAddress() override;

	virtual void Write(const std::vector<uint8_t>& data) override;

	virtual void Close() override;
};

class TcpServerManager;

class UvTcpServer : public TcpServer
{
private:
	TcpServerManager* m_manager;

	std::unique_ptr<uv_tcp_t> m_server;

	std::set<fwRefContainer<UvTcpServerStream>> m_clients;

private:
	void OnConnection(int status);

public:
	UvTcpServer(TcpServerManager* manager);

	virtual ~UvTcpServer();

	bool Listen(std::unique_ptr<uv_tcp_t>&& server);

	inline uv_tcp_t* GetServer()
	{
		return m_server.get();
	}

	inline TcpServerManager* GetManager()
	{
		return m_manager;
	}

public:
	void RemoveStream(UvTcpServerStream* stream);
};
}
