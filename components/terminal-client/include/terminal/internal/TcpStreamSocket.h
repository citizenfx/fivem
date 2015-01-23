/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <terminal/internal/Socket.h>

#include <deque>
#include <mutex>
#include <queue>

#include <ws2tcpip.h>

namespace terminal
{
class TcpStreamSocket : public StreamSocket
{
private:
	SOCKET m_socket;

	HANDLE m_eventHandle;

	enum class State
	{
		None,
		Connecting,
		Connected
	};

	State m_state;

	concurrency::task_completion_event<Result<void>> m_connectionCompletionEvent;

	std::deque<uint8_t> m_recvQueue;

	struct ReadRequest
	{
		size_t bytesTotal;
		concurrency::task_completion_event<Result<SocketReadResult>> completionEvent;
	};

	std::queue<ReadRequest> m_readRequests;

	std::mutex m_readRequestMutex;

private:
	void Tick();

	Result<void> ResultFromSocketError(int errorCode);

	void OnEvent(HANDLE handle);

	void OnReadEvent(int errorCode);

	void OnWriteEvent(int errorCode);

	void OnCloseEvent(int errorCode);

	void DistributeRecvQueue(bool finalize);

public:
	TcpStreamSocket();

	virtual ~TcpStreamSocket();

	virtual concurrency::task<Result<void>> Connect(const char* hostname, uint16_t port) override;

	virtual concurrency::task<Result<SocketReadResult>> Read(size_t bytes) override;

	virtual concurrency::task<Result<void>> Write(const std::vector<uint8_t>& data) override;
};
}