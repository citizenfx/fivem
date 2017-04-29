/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#pragma once

#include <deque>
#include <shared_mutex>

#include <TcpServer.h>

class LoopbackTcpServer;
class LoopbackTcpServerManager;

class LoopbackTcpServerStream : public net::TcpServerStream
{
private:
	// queue of packets to be received by the client
	std::deque<uint8_t> m_outQueue;

	std::recursive_mutex m_mutex;

	LoopbackTcpServer* m_server;

	SOCKET m_socket;

	bool m_nextReadWillBlock;

public:
	virtual net::PeerAddress GetPeerAddress() override;

	virtual void Write(const std::vector<uint8_t>& data) override;

	virtual void Close() override;

public:
	LoopbackTcpServerStream(LoopbackTcpServer* server, SOCKET socket);

	inline bool IsDataAvailable()
	{
		return (!m_outQueue.empty());
	}

	void HandleRead(const char* buffer, int length);

	inline bool GetNextReadWillBlock()
	{
		return m_nextReadWillBlock;
	}

	inline void SetNextReadWillBlock(bool value)
	{
		m_nextReadWillBlock = value;
	}

	int HandleWrite(char* buffer, size_t length);
};

class LoopbackTcpServer : public net::TcpServer
{
private:
	uint16_t m_port;

	LoopbackTcpServerManager* m_manager;

public:
	LoopbackTcpServer(LoopbackTcpServerManager* manager);

	inline uint16_t GetPort()
	{
		return m_port;
	}

	inline void SetPort(uint16_t port)
	{
		m_port = port;
	}

	void TriggerEvent(SOCKET socket, long event);

	fwRefContainer<LoopbackTcpServerStream> CreateConnection(SOCKET socket);
};

#include <concurrent_unordered_map.h>

class LoopbackTcpServerManager : public fwRefCountable
{
private:
	struct EventData
	{
		long subscribed;
		WSANETWORKEVENTS occurred;
	};

private:
	std::multimap<uint32_t, fwRefContainer<LoopbackTcpServer>> m_tcpServers;

	concurrency::concurrent_unordered_map<SOCKET, fwRefContainer<LoopbackTcpServerStream>> m_socketStreams;

	std::multimap<SOCKET, WSAEVENT> m_socketEvents;

	std::map<WSAEVENT, EventData> m_events;

	std::shared_mutex m_loopbackLock;

	std::map<std::pair<SOCKET, DWORD>, LPOVERLAPPED> m_threadpoolIoCallbacks;

	std::map<SOCKET, std::tuple<void*, size_t, size_t*>> m_threadpoolReadRequests;

	std::map<HANDLE, std::tuple<PTP_WIN32_IO_CALLBACK, PVOID, PTP_IO>> m_threadpoolIoHandles;

public:
	void TriggerSocketEvent(SOCKET socket, long event, size_t length = 0);

public:
	fwRefContainer<LoopbackTcpServer> RegisterTcpServer(const std::string& hostName);

	bool HasDataInSocket(SOCKET socket);

	bool GetHostByName(const char* name, hostent** outValue);

	bool GetAddrInfo(const char* name, const wchar_t* serviceName, const ADDRINFOW* hints, ADDRINFOW** addrInfo, int* outValue);

	bool GetAddrInfoEx(const char* name, const wchar_t* serviceName, const ADDRINFOEXW* hints, ADDRINFOEXW** addrInfo, int* outValue);

	bool Connect(SOCKET s, const sockaddr* name, int namelen, int* outValue);

	bool Recv(SOCKET s, char* buf, int len, int flags, int* outValue);

	bool Send(SOCKET s, const char* buf, int length, int flags, int* outValue);

	bool EventSelect(SOCKET s, WSAEVENT event, long eventMask);

	bool EnumEvents(SOCKET s, WSAEVENT event, WSANETWORKEVENTS* outEvents);

	void UntrackSocket(SOCKET s);

	void CreateThreadpoolIo(HANDLE h, PTP_WIN32_IO_CALLBACK callback, PVOID context, PTP_IO io);

	void InvokeThreadpoolIo(SOCKET s, LPOVERLAPPED overlapped, size_t readSize = 0);

	void SetThreadpoolIo(SOCKET s, DWORD mask, LPOVERLAPPED overlapped);

	void SetThreadpoolRecv(SOCKET s, LPOVERLAPPED overlapped, void* outBuffer, size_t outSize);

	inline bool OwnsSocket(SOCKET s)
	{
		return (m_socketStreams.find(s) != m_socketStreams.end());
	}
};

DECLARE_INSTANCE_TYPE(LoopbackTcpServerManager);