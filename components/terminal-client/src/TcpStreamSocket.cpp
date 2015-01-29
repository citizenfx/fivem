/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <terminal/internal/TcpStreamSocket.h>
#include <terminal/internal/WorkerThread.h>

namespace terminal
{
TcpStreamSocket::TcpStreamSocket()
	: m_state(State::None)
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);

	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	u_long nonBlocking = 1;
	ioctlsocket(m_socket, FIONBIO, &nonBlocking);

	// create connection/read/write events for the socket
	m_eventHandle = WSACreateEvent();

	// associate the events with the socket
	WSAEventSelect(m_socket, m_eventHandle, FD_CONNECT | FD_READ | FD_WRITE | FD_CLOSE);

	// register our handles
	WorkerThread* workerThread = Instance<WorkerThread>::Get();
	workerThread->RegisterTickCallback(std::bind(&TcpStreamSocket::Tick, this));
	workerThread->RegisterWaitHandle(m_eventHandle, std::bind(&TcpStreamSocket::OnEvent, this, std::placeholders::_1));
}

TcpStreamSocket::~TcpStreamSocket()
{
	// unregister our event handles
	WorkerThread* workerThread = Instance<WorkerThread>::Get();
	workerThread->UnregisterWaitHandle(m_eventHandle);

	// close the event handles
	CloseHandle(m_eventHandle);

	// TODO: remove tick callback

	// close the socket
	closesocket(m_socket);
}

void TcpStreamSocket::Tick()
{
	// meh
}

void TcpStreamSocket::OnEvent(HANDLE handle)
{
	WSANETWORKEVENTS networkEvents = { 0 };

	WSAEnumNetworkEvents(m_socket, handle, &networkEvents);

	if (networkEvents.lNetworkEvents & FD_CONNECT)
	{
		int errorCode = networkEvents.iErrorCode[FD_CONNECT_BIT];

		if (errorCode == 0)
		{
			m_connectionCompletionEvent.set(Result<void>());
		}
		else
		{
			m_connectionCompletionEvent.set(ResultFromSocketError(errorCode));
		}
	}

	if (networkEvents.lNetworkEvents & FD_READ)
	{
		int errorCode = networkEvents.iErrorCode[FD_READ_BIT];

		OnReadEvent(errorCode);
	}

	if (networkEvents.lNetworkEvents & FD_WRITE)
	{
		int errorCode = networkEvents.iErrorCode[FD_WRITE_BIT];

		OnWriteEvent(errorCode);
	}

	if (networkEvents.lNetworkEvents & FD_CLOSE)
	{
		int errorCode = networkEvents.iErrorCode[FD_CLOSE_BIT];

		OnCloseEvent(errorCode);
	}
}

void TcpStreamSocket::OnReadEvent(int errorCode)
{
	// if the read completed successfully
	if (errorCode == 0)
	{
		// receive bytes from the socket and add them to the queue
		int bytes;
		uint8_t buffer[2048];

		do
		{
			bytes = recv(m_socket, reinterpret_cast<char*>(buffer), sizeof(buffer), 0);

			if (bytes > 0)
			{
				size_t origSize = m_recvQueue.size();
				m_recvQueue.resize(origSize + bytes);

				std::copy(buffer, buffer + bytes, m_recvQueue.begin() + origSize);
			}
		} while (bytes != SOCKET_ERROR);

		// process read requests
		DistributeRecvQueue(false);
	}
	else
	{
		// fail all read requests
		m_readRequestMutex.lock();

		while (!m_readRequests.empty())
		{
			auto& readRequest = m_readRequests.front();
			readRequest.completionEvent.set(ResultFromSocketError(errorCode));

			m_readRequests.pop();
		}

		m_readRequestMutex.unlock();
	}
}

void TcpStreamSocket::OnWriteEvent(int errorCode)
{
	// if there's any write notification
	if (!m_writeNotifications.empty())
	{
		// get the first write notification from the queue
		m_writeNotificationMutex.lock();

		auto writeNotification = m_writeNotifications.front();
		m_writeNotifications.pop();

		m_writeNotificationMutex.unlock();

		// mark the task as completed
		writeNotification.set(ResultFromSocketError(errorCode));
	}
}

void TcpStreamSocket::OnCloseEvent(int errorCode)
{
	// send all data we have to any read events
	DistributeRecvQueue(true);

	// fail all remaining read requests
	m_readRequestMutex.lock();

	while (!m_readRequests.empty())
	{
		auto& readRequest = m_readRequests.front();
		readRequest.completionEvent.set((errorCode != 0) ? ResultFromSocketError(errorCode) : Result<void>(ErrorCode::EndOfStream));

		m_readRequests.pop();
	}

	m_readRequestMutex.unlock();

	// TODO: call close callbacks
}

void TcpStreamSocket::DistributeRecvQueue(bool finalize)
{
	bool satisfiable = (!m_readRequests.empty());

	while (satisfiable)
	{
		m_readRequestMutex.lock();

		ReadRequest& curRequest = m_readRequests.front();
		size_t bytesLeft = curRequest.bytesTotal;

		satisfiable = false;

		if (m_recvQueue.size() >= bytesLeft || finalize)
		{
			// max out the total bytes with the remaining size
			if (finalize)
			{
				bytesLeft = min(bytesLeft, m_recvQueue.size());
			}

			// if there's any bytes left
			if (bytesLeft > 0)
			{
				// create a new array and copy into it
				std::vector<uint8_t> byteArray;
				byteArray.resize(bytesLeft);

				std::copy(m_recvQueue.begin(), m_recvQueue.begin() + bytesLeft, byteArray.begin());

				// remove the bytes from the receive queue
				m_recvQueue.erase(m_recvQueue.begin(), m_recvQueue.begin() + bytesLeft);

				// signal the completion event
				curRequest.completionEvent.set(Result<SocketReadResult>(SocketReadResult(byteArray)));

				// and pop the request from the queue
				m_readRequests.pop();

				// mark as potentially satisfiable
				satisfiable = true;
			}
		}

		m_readRequestMutex.unlock();

		// if no requests are left, ignore satisfiable state
		if (satisfiable)
		{
			satisfiable = (!m_readRequests.empty());
		}
	};
}

Result<void> TcpStreamSocket::ResultFromSocketError(int errorCode)
{
	ErrorCode error = ErrorCode::ConnectionError;

	switch (errorCode)
	{
		case WSAECONNREFUSED:
			error = ErrorCode::ConnectionRefused;
			break;

		case WSAETIMEDOUT:
			error = ErrorCode::ConnectionTimedOut;
			break;

		case WSAHOST_NOT_FOUND:
			error = ErrorCode::InvalidHostname;
			break;
	}

	return Result<void>(error);
}

concurrency::task<Result<void>> TcpStreamSocket::Connect(const char* hostname, uint16_t port)
{
	addrinfo base = { 0 };
	base.ai_family = AF_INET;

	addrinfo* outInfo;

	int result = getaddrinfo(hostname, va("%d", port), &base, &outInfo);

	if (result == 0)
	{
		if (connect(m_socket, outInfo->ai_addr, outInfo->ai_addrlen) == SOCKET_ERROR)
		{
			result = WSAGetLastError();
		}
		
		if (result == 0 || result == WSAEWOULDBLOCK)
		{
			m_state = State::Connecting;

			m_connectionCompletionEvent = concurrency::task_completion_event<Result<void>>();

			return concurrency::task<Result<void>>(m_connectionCompletionEvent);
		}
	}
	
	return concurrency::task_from_result<Result<void>>(ResultFromSocketError(result));
}

concurrency::task<Result<SocketReadResult>> TcpStreamSocket::Read(size_t bytes)
{
	ReadRequest request;
	request.bytesTotal = bytes;

	m_readRequestMutex.lock();
	m_readRequests.push(request);
	m_readRequestMutex.unlock();

	// if we're added while there was already data waiting, try distributing
	DistributeRecvQueue(false);

	return concurrency::task<Result<SocketReadResult>>(request.completionEvent);
}

concurrency::task<Result<void>> TcpStreamSocket::Write(const std::vector<uint8_t>& data)
{
	int res = send(m_socket, reinterpret_cast<const char*>(&data[0]), data.size(), 0);

	if (res == SOCKET_ERROR)
	{
		int errorCode = WSAGetLastError();

		if (errorCode != WSAEWOULDBLOCK)
		{
			trace("[Terminal] send() error %d\n", errorCode);

			return concurrency::task_from_result(ResultFromSocketError(errorCode));
		}
	}

	concurrency::task_completion_event<Result<void>> completionEvent;

	m_writeNotificationMutex.lock();
	m_writeNotifications.push(completionEvent);
	m_writeNotificationMutex.unlock();

	return concurrency::task<Result<void>>(completionEvent);
}
}