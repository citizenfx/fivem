/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <Hooking.h>
#include <ros/LoopbackTcpServer.h>
#include <IteratorView.h>

#include <LaunchMode.h>
#include <CfxSubProcess.h>

#include <CL2LaunchMode.h>
#include <HostSharedData.h>
#include <CrossBuildRuntime.h>

#include <ROSSuffix.h>

#include <Error.h>

struct LauncherState
{
	volatile DWORD pid = 0;
};

#ifdef GTA_FIVE
#define PIPE_NAME L"\\\\.\\pipe\\GTAVLauncher_Pipe"
#define PIPE_NAME_NARROW "\\\\.\\pipe\\GTAVLauncher_Pipe"
#else
#define PIPE_NAME L"\\\\.\\pipe\\RDR2Launcher_Pipe"
#define PIPE_NAME_NARROW "\\\\.\\pipe\\RDR2Launcher_Pipe"
#endif

#define REMOVE_EVENT_INSTANTLY (1 << 16)

using scoped_lock = std::shared_lock<std::shared_mutex>;
using scoped_write_lock = std::unique_lock<std::shared_mutex>;

static int(__stdcall* g_oldGetAddrInfoW)(const wchar_t*, const wchar_t*, const ADDRINFOW*, ADDRINFOW**);
static int(__stdcall* g_oldGetAddrInfoExW)(const wchar_t* name, const wchar_t* serviceName, DWORD namespace_, LPGUID nspId, const ADDRINFOEXW* hints,
										   ADDRINFOEXW** result, timeval* timeout, LPOVERLAPPED overlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE completionRoutine,
										   LPHANDLE nameHandle);
static int(__stdcall* g_oldGetAddrInfo)(const char*, const char*, const addrinfo*, addrinfo**);

LoopbackTcpServerStream::LoopbackTcpServerStream(LoopbackTcpServer* server, SOCKET socket)
	: m_server(server), m_socket(socket), m_nextReadWillBlock(false)
{

}

void LoopbackTcpServerStream::HandleRead(const char* buffer, int length)
{
	// create a new dummy buffer to send to the read callback
	auto bufStart = reinterpret_cast<const uint8_t*>(buffer);
	std::vector<uint8_t> data(bufStart, bufStart + length);

	// handle the read
	if (GetReadCallback())
	{
		GetReadCallback()(data);
	}
}

int LoopbackTcpServerStream::HandleWrite(char* buffer, size_t length)
{
	int toRead = fwMin(m_outQueue.size(), length);

	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	std::copy(m_outQueue.begin(), m_outQueue.begin() + toRead, reinterpret_cast<uint8_t*>(buffer));
	m_outQueue.erase(m_outQueue.begin(), m_outQueue.begin() + toRead);

	//trace("recv %d bytes\n", toRead);

	return toRead;
}

net::PeerAddress LoopbackTcpServerStream::GetPeerAddress()
{
	return net::PeerAddress::FromString("127.0.0.1:65535", 65535, net::PeerAddress::LookupType::NoResolution).get();
}

void LoopbackTcpServerStream::Write(const std::vector<uint8_t>& data, TCompleteCallback&& onComplete)
{
	std::unique_lock<std::recursive_mutex> lock(m_mutex);

	size_t oldSize = m_outQueue.size();

	m_outQueue.resize(oldSize + data.size());
	std::copy(data.begin(), data.end(), m_outQueue.begin() + oldSize);

	m_server->TriggerEvent(m_socket, FD_READ);

	if (onComplete)
	{
		onComplete(true);
	}
}

void LoopbackTcpServerStream::Close()
{
	if (GetCloseCallback())
	{
		GetCloseCallback()();
	}
}

void LoopbackTcpServerStream::StartConnectionTimeout(std::chrono::duration<uint64_t, std::milli> timeout)
{
	// a connection timeout is not required for the loopback tcp server stream, because its only on the client and not exposed to the network
}

LoopbackTcpServer::LoopbackTcpServer(LoopbackTcpServerManager* manager, const std::string& hostName)
	: m_port(0), m_manager(manager), m_hostName(hostName)
{

}

fwRefContainer<LoopbackTcpServerStream> LoopbackTcpServer::CreateConnection(SOCKET socket)
{
	// create the stream, invoke connection callback and return
	fwRefContainer<LoopbackTcpServerStream> stream = new LoopbackTcpServerStream(this, socket);

	if (GetConnectionCallback())
	{
		GetConnectionCallback()(stream);
	}

	return stream;
}

void LoopbackTcpServer::TriggerEvent(SOCKET socket, long event)
{
	return m_manager->TriggerSocketEvent(socket, event);
}

bool LoopbackTcpServerManager::GetHostByName(const char* name, hostent** outValue)
{
	scoped_lock lock(m_loopbackLock);

	// hash the hostname for lookup in the server dictionary
	uint32_t hostHash = HashString(name);

	hostHash &= 0xFFFFFF; // for adding the 127.x netname

	// find the matching server
	auto it = m_tcpServers.find(hostHash);

	if (it != m_tcpServers.end())
	{
		// return a fake hostent
		static thread_local hostent hostEntity;
		static thread_local in_addr hostEntityAddr;
		hostEntity.h_name = strdup(name);
		hostEntity.h_addrtype = AF_INET;
		hostEntity.h_aliases = nullptr;
		hostEntity.h_length = sizeof(sockaddr_in);
		hostEntity.h_addr_list = reinterpret_cast<char**>(&hostEntityAddr);

		// 127.0.0.0/8 network
		hostEntityAddr.s_addr = htonl(hostHash | (127 << 24));

		// mm
		*outValue = &hostEntity;

		return true;
	}

	return false;
}

bool LoopbackTcpServerManager::GetAddrInfoA(const char* name, const char* serviceName, const addrinfo* hints, addrinfo** addrInfo, int* outValue)
{
	scoped_lock lock(m_loopbackLock);

	// hash the hostname for lookup in the server dictionary
	uint32_t hostHash = HashString(name);

	hostHash &= 0xFFFFFF; // for adding the 127.x netname

	// find the matching server
	auto it = m_tcpServers.find(hostHash);

	if (it != m_tcpServers.end())
	{
		// we can't fill structs directly as we can't hook FreeAddrInfoW (hook checks) and that'll mean instant heap corruption
		// therefore, we parse it into an IP and return that *natively*

		uint32_t addr = hostHash | (127 << 24);

		*outValue = g_oldGetAddrInfo(va("%u.%u.%u.%u", addr >> 24, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF), serviceName, hints, addrInfo);

		return true;
	}

	if (strcmp(name, "ros.citizenfx.internal") == 0)
	{
		*outValue = g_oldGetAddrInfo("prod.ros.rockstargames.com", serviceName, hints, addrInfo);

		return true;
	}

	return false;
}

bool LoopbackTcpServerManager::GetAddrInfo(const char* name, const wchar_t* serviceName, const ADDRINFOW* hints, ADDRINFOW** addrInfo, int* outValue)
{
	//trace("getaddrinfo %s\n", name);

	scoped_lock lock(m_loopbackLock);

	// hash the hostname for lookup in the server dictionary
	uint32_t hostHash = HashString(name);

	hostHash &= 0xFFFFFF; // for adding the 127.x netname

	// find the matching server
	auto it = m_tcpServers.find(hostHash);

	if (it != m_tcpServers.end())
	{
		/*sockaddr_in* inAddr = new sockaddr_in;
		inAddr->sin_family = AF_INET;
		inAddr->sin_addr.s_addr = htonl(hostHash | (127 << 24));
		inAddr->sin_port = 0;
		memset(inAddr->sin_zero, 0, sizeof(inAddr->sin_zero));

		ADDRINFOW* info = new ADDRINFOW;
		info->ai_addr = reinterpret_cast<sockaddr*>(inAddr);
		info->ai_addrlen = sizeof(sockaddr_in);
		info->ai_canonname = nullptr;// reinterpret_cast<wchar_t*>(strdup(name)); // yol
		info->ai_family = AF_INET;
		info->ai_flags = 0;
		info->ai_next = nullptr;
		info->ai_protocol = 0;
		info->ai_socktype = 0;

		ADDRINFOW* infoNext = new ADDRINFOW(*info);
		info->ai_next = infoNext;

		*addrInfo = info;
		
		*outValue = 0;*/

		// we can't do the above as we can't hook FreeAddrInfoW (hook checks) and that'll mean instant heap corruption
		// therefore, we parse it into an IP and return that *natively*

		uint32_t addr = hostHash | (127 << 24);

		*outValue = g_oldGetAddrInfoW(va(L"%u.%u.%u.%u", addr >> 24, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF), serviceName, hints, addrInfo);

		return true;
	}

	if (strcmp(name, "ros.citizenfx.internal") == 0)
	{
		*outValue = g_oldGetAddrInfoW(L"prod.ros.rockstargames.com", serviceName, hints, addrInfo);

		return true;
	}

	return false;
}

bool LoopbackTcpServerManager::GetAddrInfoEx(const char* name, const wchar_t* serviceName,  const ADDRINFOEXW* hints, ADDRINFOEXW** addrInfo, int* outValue)
{
	scoped_lock lock(m_loopbackLock);

	// hash the hostname for lookup in the server dictionary
	uint32_t hostHash = HashString(name);

	hostHash &= 0xFFFFFF; // for adding the 127.x netname

	// find the matching server
	auto it = m_tcpServers.find(hostHash);

	if (it != m_tcpServers.end())
	{
		uint32_t addr = hostHash | (127 << 24);

		*outValue = g_oldGetAddrInfoExW(va(L"%u.%u.%u.%u", addr >> 24, (addr >> 16) & 0xFF, (addr >> 8) & 0xFF, addr & 0xFF), serviceName, NS_DNS, nullptr, hints, addrInfo, nullptr, nullptr, nullptr, nullptr);

		return true;
	}

	if (strcmp(name, "ros.citizenfx.internal") == 0)
	{
		*outValue = g_oldGetAddrInfoExW(L"prod.ros.rockstargames.com", serviceName, NS_DNS, nullptr, hints, addrInfo, nullptr, nullptr, nullptr, nullptr);

		return true;
	}

	return false;
}

bool LoopbackTcpServerManager::Connect(SOCKET s, const sockaddr* name, int namelen, int* outValue)
{
	// if the address type is AF_INET...
	if (name->sa_family == AF_INET)
	{
		const sockaddr_in* inAddr = reinterpret_cast<const sockaddr_in*>(name); // it's a sockaddr_in

		// ... and the address matches a hashed service...
		uint32_t hostHash = ntohl(inAddr->sin_addr.s_addr);
		hostHash &= 0xFFFFFF;

		uint16_t port = ntohs(inAddr->sin_port);

		auto it = fx::GetIteratorView(m_tcpServers.equal_range(hostHash));

		for (auto& serverEntry : it)
		{
			// connect the specific TCP server to the fake peer
			fwRefContainer<LoopbackTcpServer> server = serverEntry.second;
			uint16_t serverPort = server->GetPort();
			
			if (serverPort == 0 || serverPort == port)
			{
				if (m_socketStreams.find(s) == m_socketStreams.end())
				{
					m_socketStreams.insert({ s, server->CreateConnection(s) });
				}
				else
				{
					m_socketStreams[s] = server->CreateConnection(s);
				}

				// pass the return values correctly
				*outValue = SOCKET_ERROR;

				WSASetLastError(WSAEWOULDBLOCK);

				TriggerSocketEvent(s, FD_CONNECT);
				TriggerSocketEvent(s, FD_WRITE);

				WSASetLastError(WSAEWOULDBLOCK);

				return true;
			}
		}
	}

	// reflect the call to the previous hook
	return false;
}

bool LoopbackTcpServerManager::HasDataInSocket(SOCKET s)
{
	// early out if this doesn't match any of our sockets
	auto it = m_socketStreams.find(s);

	if (it == m_socketStreams.end() || !it->second.GetRef())
	{
		return false;
	}

	// get the stream
	fwRefContainer<LoopbackTcpServerStream> stream = it->second;

	return stream->IsDataAvailable();
}

bool LoopbackTcpServerManager::Recv(SOCKET s, char* buf, int length, int flags, int* outValue)
{
	fwRefContainer<LoopbackTcpServerStream> stream;

	{
		// early out if this doesn't match any of our sockets
		auto it = m_socketStreams.find(s);

		if (it == m_socketStreams.end() || !it->second.GetRef())
		{
			return false;
		}

		// get the stream
		stream = it->second;
	}

	if (!stream->GetNextReadWillBlock() && stream->IsDataAvailable())
	{
		*outValue = stream->HandleWrite(buf, length);
	}
	else
	{
		stream->SetNextReadWillBlock(false);

		*outValue = -1;

		WSASetLastError(WSAEWOULDBLOCK);
	}

	return true;
}

bool LoopbackTcpServerManager::Send(SOCKET s, const char* buf, int length, int flags, int* outValue)
{
	fwRefContainer<LoopbackTcpServerStream> stream;

	{
		// early out if this doesn't match any of our sockets
		auto it = m_socketStreams.find(s);

		if (it == m_socketStreams.end() || !it->second.GetRef())
		{
			return false;
		}

		// get the stream
		stream = it->second;
	}

	stream->HandleRead(buf, length);

	TriggerSocketEvent(s, FD_WRITE, length);

	*outValue = length;

	return true;
}

void LoopbackTcpServerManager::UntrackSocket(SOCKET s)
{
	auto it = m_socketStreams.find(s);

	if (it != m_socketStreams.end() && it->second.GetRef())
	{
		it->second->Close();
		it->second = nullptr;
	}
}

bool LoopbackTcpServerManager::EnumEvents(SOCKET s, WSAEVENT event, WSANETWORKEVENTS* outEvents)
{
	scoped_lock lock(m_loopbackLock);

	auto range = fx::GetIteratorView(m_socketEvents.equal_range(s));

	for (auto& ev : range)
	{
		auto& eventData = m_events[ev.second];

		outEvents->lNetworkEvents |= eventData.occurred.lNetworkEvents;
		
		for (int i = 0; i < 10; i++)
		{
			if (eventData.occurred.lNetworkEvents & (1 << i))
			{
				outEvents->iErrorCode[i] = eventData.occurred.iErrorCode[i];
			}
		}

		// is this correct behavior? (reset event)
		eventData.occurred.lNetworkEvents = 0;
	}

	if (event != 0 && event != INVALID_HANDLE_VALUE)
	{
		ResetEvent(event);
	}

	return true;
}

bool LoopbackTcpServerManager::EventSelect(SOCKET s, WSAEVENT event, long eventMask)
{
	scoped_write_lock lock(m_loopbackLock);

	m_socketEvents.insert({ s, event });

	EventData ev = { 0 };
	ev.subscribed = eventMask;

	m_events[event] = ev;

	// hack: if event mask contains FD_READ and data is available, trigger immediately
	if (eventMask & FD_READ)
	{
		auto it = m_socketStreams.find(s);

		if (it != m_socketStreams.end() && it->second.GetRef())
		{
			fwRefContainer<LoopbackTcpServerStream> stream = it->second;

			if (stream->IsDataAvailable())
			{
				TriggerSocketEvent(s, FD_READ);

				// worse hack: Chromium does FD_READ and a recv() in the same call, but if recv() instantly returns data it ignores the data - set a flag in the stream to make the next recv block
				stream->SetNextReadWillBlock(true);
			}
		}
	}

	return true;
}

void LoopbackTcpServerManager::TriggerSocketEvent(SOCKET socket, long event, size_t length)
{
	auto range = fx::GetIteratorView(m_socketEvents.equal_range(socket));

	for (auto& ev : range)
	{
		auto& eventData = m_events[ev.second];

		if (eventData.subscribed & event)
		{
			eventData.occurred.lNetworkEvents |= event;
			
			for (int i = 0; i < 10; i++)
			{
				if (event & (1 << i))
				{
					eventData.occurred.iErrorCode[i] = 0;
				}
			}

			WSASetEvent(ev.second);

			if (eventData.subscribed & REMOVE_EVENT_INSTANTLY)
			{
				m_events[ev.second].subscribed = 0;
			}
		}
	}

	// find a proper overlapped for the threadpool io
	auto overlapped = m_threadpoolIoCallbacks.find({ socket, event });

	if (overlapped != m_threadpoolIoCallbacks.end())
	{
		if (event & FD_READ)
		{
			auto readIt = m_threadpoolReadRequests.find(socket);

			if (readIt != m_threadpoolReadRequests.end())
			{
				auto& readEvent = readIt->second;

				Recv(socket, reinterpret_cast<char*>(std::get<void*>(readEvent)), std::get<size_t>(readEvent), 0, reinterpret_cast<int*>(std::get<ULONG_PTR*>(readEvent)));

				m_threadpoolReadRequests.erase(readIt);
			}
		}

		InvokeThreadpoolIo(socket, overlapped->second, (length == 0) ? overlapped->second->InternalHigh : length);
	}
}

void LoopbackTcpServerManager::CreateThreadpoolIo(HANDLE h, PTP_WIN32_IO_CALLBACK callback, PVOID context, PTP_IO io)
{
	m_threadpoolIoHandles[h] = std::tuple<PTP_WIN32_IO_CALLBACK, PVOID, PTP_IO>{ callback, context, io };
}

void LoopbackTcpServerManager::InvokeThreadpoolIo(SOCKET s, LPOVERLAPPED overlapped, size_t readSize)
{
	auto cbDataIt = m_threadpoolIoHandles.find(reinterpret_cast<HANDLE>(s));

	if (cbDataIt == m_threadpoolIoHandles.end())
	{
		return;
	}

	auto& cbData = cbDataIt->second;

	char instancePadding[128] = { 0 };
	PTP_CALLBACK_INSTANCE instance = reinterpret_cast<PTP_CALLBACK_INSTANCE>(instancePadding);

	// queue on the thread pool like a real man
	std::function<void()>* cb = new std::function<void()>([=] ()
	{
		std::get<PTP_WIN32_IO_CALLBACK>(cbData)(instance, std::get<PVOID>(cbData), overlapped, ERROR_SUCCESS, readSize, std::get<PTP_IO>(cbData));
	});

	QueueUserWorkItem([] (LPVOID userData) -> DWORD
	{
		auto cb = reinterpret_cast<std::function<void()>*>(userData);

		__try
		{
			(*cb)();
		}
		__except (EXCEPTION_EXECUTE_HANDLER)
		{

		}

		delete cb;

		return 0;
	}, cb, 0);
}

void LoopbackTcpServerManager::SetThreadpoolIo(SOCKET s, DWORD mask, LPOVERLAPPED overlapped)
{
	m_threadpoolIoCallbacks[{ s, mask }] = overlapped;
}

void LoopbackTcpServerManager::SetThreadpoolRecv(SOCKET s, LPOVERLAPPED overlapped, void* outBuffer, size_t outSize)
{
	m_threadpoolReadRequests[s] = std::tuple<void*, size_t, ULONG_PTR*>{ outBuffer, outSize, &overlapped->InternalHigh };
	m_threadpoolIoCallbacks[{s, FD_READ}] = overlapped;
}

fwRefContainer<LoopbackTcpServer> LoopbackTcpServerManager::RegisterTcpServer(const std::string& hostName)
{
	scoped_write_lock lock(m_loopbackLock);

	fwRefContainer<LoopbackTcpServer> server = new LoopbackTcpServer(this, hostName);
	m_tcpServers.insert({ HashString(hostName.c_str()) & 0xFFFFFF, server });

	return server;
}

static LoopbackTcpServerManager* g_manager;

static int(__stdcall* g_oldConnect)(SOCKET, const sockaddr*, int);
static int(__stdcall* g_oldSend)(SOCKET, const char*, int, int);
static int(__stdcall* g_oldRecv)(SOCKET, char*, int, int);
static hostent*(__stdcall* g_oldGetHostByName)(const char*);
static int(__stdcall* g_oldGetSockName)(SOCKET, sockaddr*, int*);
static int(__stdcall* g_oldGetPeerName)(SOCKET, sockaddr*, int*);
static int(__stdcall* g_oldSetSockOpt)(SOCKET, int, int, const char*, int);
static void(__stdcall* g_oldFreeAddrInfo)(addrinfo*);
static void(__stdcall* g_oldFreeAddrInfoW)(ADDRINFOW*);
static int(__stdcall* g_oldSelect)(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout);
static int(__stdcall* g_oldCloseSocket)(SOCKET);
static int(__stdcall* g_oldLookupService)(LPWSAQUERYSET, DWORD, LPHANDLE);
static int(__stdcall* g_oldEnumEvents)(SOCKET, WSAEVENT, LPWSANETWORKEVENTS);
static int(__stdcall* g_oldEventSelect)(SOCKET, WSAEVENT, long);
static int(__stdcall* g_oldWSARecv)(SOCKET, LPWSABUF, DWORD, LPDWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
static int(__stdcall* g_oldWSASend)(SOCKET, LPWSABUF, DWORD, LPDWORD, DWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);
static int(__stdcall* g_oldWSAIoctl)(SOCKET, DWORD, LPVOID, DWORD, LPVOID, DWORD, LPDWORD, LPWSAOVERLAPPED, LPWSAOVERLAPPED_COMPLETION_ROUTINE);

#include <filesystem>
#include <psapi.h>

namespace fs = std::filesystem;

struct ModuleData
{
	using TSet = std::map<uintptr_t, uintptr_t>;

	const TSet dataSet;

	ModuleData()
		: dataSet(CreateSet())
	{
		
	}

	bool IsInSet(void* address)
	{
		uintptr_t ptr = (uintptr_t)address;

		auto it = dataSet.upper_bound(ptr);

		if (it != dataSet.begin())
		{
			it--;

			if (ptr >= it->first && ptr < it->second)
			{
				/*wchar_t fn[260];
				GetModuleFileName((HMODULE)it->first, fn, sizeof(fn) / sizeof(*fn));

				trace("Shouldn't be hooked - %016llx >= %016llx & < %016llx (%s)\n", ptr, it->first, it->second, ToNarrow(fn));*/

				return true;
			}
		}

		return false;
	}

private:
	TSet CreateSet()
	{
		TSet set;

		for (const auto& rootTree : { L"", L"bin" })
		{
			fs::path plugins_path(MakeRelativeCitPath(rootTree));

			try
			{
				fs::directory_iterator it(plugins_path), end;

				while (it != end)
				{
					// gta-net-five hooks select() after us, so our hook will think any caller is Cfx
					if (it->path().extension() == ".dll" && it->path().filename() != "gta-net-five.dll" && it->path().filename() != "gta-net-rdr3.dll")
					{
						HMODULE hMod = GetModuleHandle(it->path().c_str());

						if (hMod)
						{
							MODULEINFO mi;
							GetModuleInformation(GetCurrentProcess(), hMod, &mi, sizeof(mi));

							set.insert({ (uintptr_t)hMod, (uintptr_t)hMod + mi.SizeOfImage });
						}
					}

					it++;
				}
			}
			catch (std::exception& e)
			{
			}
		}

		return std::move(set);
	}
};

static bool ShouldBeHooked(void* returnAddress)
{
	static ModuleData moduleData;

	if (moduleData.IsInSet(returnAddress))
	{
		return false;
	}

	return true;
}

static int __stdcall EP_Connect(SOCKET s, const sockaddr* name, int namelen)
{
	int outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !g_manager->Connect(s, name, namelen, &outValue))
	{
		return g_oldConnect(s, name, namelen);
	}

	return outValue;
}

static int __stdcall EP_Select(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout)
{
	if (!ShouldBeHooked(_ReturnAddress()))
	{
		return g_oldSelect(nfds, readfds, writefds, exceptfds, timeout);
	}

	std::vector<SOCKET> addReadfds;

#define REMOVE_FD(x) memmove(&x->fd_array[i + 1], &x->fd_array[i], x->fd_count - i - 1); \
	x->fd_count -= 1; \
	nfds--;

	if (readfds)
	{
		for (int i = 0; i < readfds->fd_count; i++)
		{
			SOCKET fd = readfds->fd_array[i];

			if (g_manager->HasDataInSocket(fd))
			{
				REMOVE_FD(readfds);

				addReadfds.push_back(fd);
			}
		}
	}

	fd_set oldwritefds = { 0 };
	FD_ZERO(&oldwritefds);

	if (writefds)
	{
		for (int i = 0; i < writefds->fd_count; i++)
		{
            SOCKET fd = writefds->fd_array[i];

            if (g_manager->OwnsSocket(fd))
            {
                REMOVE_FD(writefds);

                FD_SET(writefds->fd_array[i], &oldwritefds);
            }
		}
	}

	if ((readfds && readfds->fd_count) || (writefds && writefds->fd_count) || (exceptfds && exceptfds->fd_count))
	{
		timeval timeout_ = (timeout) ? *timeout : timeval{ INT_MAX, 0};

		if (oldwritefds.fd_count)
		{
			Sleep(15);

			timeout_.tv_sec = 0;
			timeout_.tv_usec = 0;
		}

		nfds = g_oldSelect(nfds, readfds, writefds, exceptfds, &timeout_) + oldwritefds.fd_count;
	}

	for (auto& fd : addReadfds)
	{
		FD_SET(fd, readfds);
		nfds++;
	}

    if (exceptfds)
    {
        FD_ZERO(exceptfds);
    }

	for (int i = 0; i < oldwritefds.fd_count; i++)
	{
		FD_SET(oldwritefds.fd_array[i], writefds);
	}

	return nfds;
}

static int __stdcall EP_Recv(SOCKET s, char* buf, int length, int flags)
{
	int outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !g_manager->Recv(s, buf, length, flags, &outValue))
	{
		return g_oldRecv(s, buf, length, flags);
	}

	return outValue;
}

static int __stdcall EP_Send(SOCKET s, const char* buf, int length, int flags)
{
	int outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !g_manager->Send(s, buf, length, flags, &outValue))
	{
		return g_oldSend(s, buf, length, flags);
	}

	return outValue;
}

static int __stdcall EP_WSARecv(SOCKET s, LPWSABUF buffers, DWORD bufferCount, LPDWORD bytesRead, LPDWORD flags, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE cr)
{
	if (ShouldBeHooked(_ReturnAddress()) && g_manager->OwnsSocket(s))
	{
		// we don't handle any other case yet
		assert(bufferCount == 1);
		
		if (!overlapped)
		{
			assert(bytesRead);

			int dummy;
			g_manager->Recv(s, buffers[0].buf, buffers[0].len, 0, &dummy);

			if (dummy >= 0)
			{
				*bytesRead = dummy;
			}

			return (dummy >= 0) ? 0 : SOCKET_ERROR;
		}

		// check if there's any data to initially read
		if (g_manager->HasDataInSocket(s))
		{
			int dummy;
			g_manager->Recv(s, buffers[0].buf, buffers[0].len, 0, &dummy);

			if (bytesRead)
			{
				*bytesRead = dummy;
			}

			overlapped->InternalHigh = dummy;

			return 0;
		}

		SetEvent(overlapped->hEvent);

		// queue an event for when the read completes
		g_manager->SetThreadpoolRecv(s, overlapped, buffers[0].buf, buffers[0].len);

		WSASetLastError(WSA_IO_PENDING);

		return SOCKET_ERROR;
	}

	return g_oldWSARecv(s, buffers, bufferCount, bytesRead, flags, overlapped, cr);
}

static int __stdcall EP_WSASend(SOCKET s, LPWSABUF buffers, DWORD bufferCount, LPDWORD bytesWritten, DWORD flags, LPWSAOVERLAPPED overlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE cr)
{
	if (ShouldBeHooked(_ReturnAddress()) && g_manager->OwnsSocket(s))
	{
		int sent;

		// queue an event for when the read completes
		g_manager->SetThreadpoolIo(s, FD_WRITE, overlapped);

		// 0 bytes written initially
		*bytesWritten = 0;

		for (int i = 0; i < bufferCount; i++)
		{
			g_manager->Send(s, buffers[i].buf, buffers[i].len, flags, &sent);

			if (bytesWritten != nullptr)
			{
				*bytesWritten += sent;
			}

			// NOTE: this doesn't schedule the APC completion routine
		}

		if (overlapped)
		{
			SetEvent(overlapped->hEvent);

			overlapped->Internal = ERROR_SUCCESS;
			overlapped->InternalHigh = *bytesWritten;

			WSASetLastError(WSA_IO_PENDING);
		}

		return (overlapped) ? SOCKET_ERROR : 0;
	}

	return g_oldWSASend(s, buffers, bufferCount, bytesWritten, flags, overlapped, cr);
}

static int EP_GetSockNameInternal(SOCKET s, sockaddr* name, int* namelen)
{
	sockaddr_in* inName = reinterpret_cast<sockaddr_in*>(name);
	inName->sin_family = AF_INET;
	inName->sin_addr.s_addr = inet_addr("127.0.0.1");
	inName->sin_port = htons(60000);
	memset(inName->sin_zero, 0, sizeof(inName->sin_zero));

	*namelen = sizeof(sockaddr_in);

	return 0;
}

static int __stdcall EP_GetSockName(SOCKET s, sockaddr* name, int* namelen)
{
	if (ShouldBeHooked(_ReturnAddress()) && g_manager->OwnsSocket(s))
	{
		return EP_GetSockNameInternal(s, name, namelen);
	}

	return g_oldGetSockName(s, name, namelen);
}

static int __stdcall EP_GetPeerName(SOCKET s, sockaddr* name, int* namelen)
{
	if (ShouldBeHooked(_ReturnAddress()) && g_manager->OwnsSocket(s))
	{
		return EP_GetSockNameInternal(s, name, namelen);
	}

	return g_oldGetPeerName(s, name, namelen);
}

static int(__stdcall* g_oldGetHostName)(char* name, int namelen);

static int __stdcall EP_GetHostName(char* name, int namelen)
{
	DWORD len = namelen;
	GetComputerNameA(name, &len);
	return 0;
}

#include <mswsock.h>

static int __stdcall EP_SetSockOpt(SOCKET s, int level, int optname, const char* optval, int optlen)
{
	if (ShouldBeHooked(_ReturnAddress()) && g_manager->OwnsSocket(s))
	{
		if (level == SOL_SOCKET && optname == SO_UPDATE_CONNECT_CONTEXT)
		{
			return 0;
		}
	}

	return g_oldSetSockOpt(s, level, optname, optval, optlen);
}

static hostent* __stdcall EP_GetHostByName(const char* name)
{
	hostent* outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !g_manager->GetHostByName(name, &outValue))
	{
		return g_oldGetHostByName(name);
	}

	return outValue;
}

static int __stdcall EP_GetAddrInfo(const char* name, const char* serviceName, const addrinfo* hints, addrinfo** result)
{
	int outValue;

	// we aren't doing a ShouldBeHooked check here, since this will be used for e.g. `ros.citizenfx.internal` which we *do* want to hook
	if (!g_manager->GetAddrInfoA(name, serviceName, hints, result, &outValue))
	{
		return g_oldGetAddrInfo(name, serviceName, hints, result);
	}

	return outValue;
}

static int __stdcall EP_GetAddrInfoW(const wchar_t* name, const wchar_t* serviceName, const ADDRINFOW* hints, ADDRINFOW** result)
{
	int outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !name || !g_manager->GetAddrInfo(ToNarrow(name).c_str(), serviceName, hints, result, &outValue))
	{
		return g_oldGetAddrInfoW(name, serviceName, hints, result);
	}

	return outValue;
}

static int __stdcall EP_GetAddrInfoExW(const wchar_t* name, const wchar_t* serviceName, DWORD namespace_, LPGUID nspId, const ADDRINFOEXW* hints,
									   ADDRINFOEXW** result, timeval* timeout, LPOVERLAPPED overlapped, LPLOOKUPSERVICE_COMPLETION_ROUTINE completionRoutine,
									   LPHANDLE nameHandle)
{
	int outValue;

	if (overlapped || !ShouldBeHooked(_ReturnAddress()) || !g_manager->GetAddrInfoEx(ToNarrow(name).c_str(), serviceName, hints, result, &outValue))
	{
		return g_oldGetAddrInfoExW(name, serviceName, namespace_, nspId, hints, result, timeout, overlapped, completionRoutine, nameHandle);
	}

	return outValue;
}

static void __stdcall EP_FreeAddrInfo(addrinfo* info)
{
	// no-op for now
}

static void __stdcall EP_FreeAddrInfoW(ADDRINFOW* info)
{
	// no-op for now
	trace("FreeAddrInfoW: stub\n");
}

static int __stdcall EP_WSALookupServiceBeginW(LPWSAQUERYSET restrictions, DWORD cf, LPHANDLE outHandle)
{
	return g_oldLookupService(restrictions, cf, outHandle);
}

static int __stdcall EP_WSAEventSelect(SOCKET socket, WSAEVENT event, long events)
{
	int retval = g_oldEventSelect(socket, event, events);

	if (ShouldBeHooked(_ReturnAddress()) && retval == 0)
	{
		g_manager->EventSelect(socket, event, events);
	}

	return retval;
}

static int __stdcall EP_WSAEnumNetworkEvents(SOCKET socket, WSAEVENT event, LPWSANETWORKEVENTS events)
{
	memset(events, 0, sizeof(*events));

	int retval = g_oldEnumEvents(socket, event, events);
	
	if (ShouldBeHooked(_ReturnAddress()) && retval == 0)
	{
		g_manager->EnumEvents(socket, event, events);
	}

	return retval;
}

static int __stdcall EP_CloseSocket(SOCKET sock)
{
	g_manager->UntrackSocket(sock);

	return g_oldCloseSocket(sock);
}

#include <mswsock.h>

static LPFN_CONNECTEX g_oldConnectEx;

static BOOL __stdcall EP_ConnectEx(SOCKET s, const sockaddr* name, int namelen, void* sendBuffer, DWORD sendLength, LPDWORD bytesSent, LPOVERLAPPED overlapped)
{
	int outValue;

	if (!ShouldBeHooked(_ReturnAddress()) || !g_manager->Connect(s, name, namelen, &outValue))
	{
		return g_oldConnectEx(s, name, namelen, sendBuffer, sendLength, bytesSent, overlapped);
	}

	if (sendBuffer)
	{
		g_manager->Send(s, reinterpret_cast<char*>(sendBuffer), sendLength, 0, &outValue);
	}

	if (overlapped)
	{
		g_manager->InvokeThreadpoolIo(s, overlapped);
	}

	WSASetLastError(WSA_IO_PENDING);

	return FALSE;
}

static int __stdcall EP_WSAIoctl(SOCKET sock, DWORD dwIoControlCode, LPVOID lpvInBuffer, DWORD cbInBuffer, LPVOID lpvOutBuffer, DWORD cbOutBuffer,
								 LPDWORD lpcbBytesReturned, LPWSAOVERLAPPED lpOverlapped, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
	int retval = g_oldWSAIoctl(sock, dwIoControlCode, lpvInBuffer, cbInBuffer, lpvOutBuffer, cbOutBuffer, lpcbBytesReturned, lpOverlapped, lpCompletionRoutine);

	// to hook ConnectEx (for wininet)
	if (dwIoControlCode == SIO_GET_EXTENSION_FUNCTION_POINTER)
	{
		static GUID comparisonId = WSAID_CONNECTEX;

		if (memcmp(lpvInBuffer, &comparisonId, sizeof(comparisonId)) == 0)
		{
			LPFN_CONNECTEX* outFunction = reinterpret_cast<LPFN_CONNECTEX*>(lpvOutBuffer);

			g_oldConnectEx = *outFunction;
			*outFunction = EP_ConnectEx;
		}
	}

	return retval;
}

static PTP_IO(__stdcall* g_oldCreateThreadpoolIo)(HANDLE handle, PTP_WIN32_IO_CALLBACK cb, PVOID context, PTP_CALLBACK_ENVIRON env);

static PTP_IO __stdcall EP_CreateThreadpoolIo(HANDLE handle, PTP_WIN32_IO_CALLBACK cb, PVOID context, PTP_CALLBACK_ENVIRON env)
{
	PTP_IO io = g_oldCreateThreadpoolIo(handle, cb, context, env);

	if (ShouldBeHooked(_ReturnAddress()))
	{
		g_manager->CreateThreadpoolIo(handle, cb, context, io);
	}

	return io;
}

static BOOL(__stdcall* g_oldCallbackMayRunLong)(PTP_CALLBACK_INSTANCE);

static BOOL __stdcall EP_CallbackMayRunLong(PTP_CALLBACK_INSTANCE cb)
{
	// no-op (for now?) - to fix windows being nutty

	return TRUE;
}

// TODO: CloseThreadpoolIo?

#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

#include <sstream>

#include <EnvironmentBlockHelpers.h>

static BOOL(__stdcall *g_oldCreateProcessW)(const wchar_t* applicationName, wchar_t* commandLine, SECURITY_ATTRIBUTES* processAttributes, SECURITY_ATTRIBUTES* threadAttributes,
								  BOOL inheritHandles, DWORD creationFlags, void* environment, const wchar_t* currentDirectory, STARTUPINFOW* startupInfo,
								  PROCESS_INFORMATION* information);

int g_rosParentPid;
extern volatile bool g_launchDone;

std::vector<int> g_subProcessHandles;

#include <shlwapi.h>

extern void SubprocessPipe(const std::wstring& s);

static std::set<HANDLE> g_foregroundProcesses;
static std::mutex g_foregroundProcessesMutex;

extern "C" BOOL WINAPI __SetAdditionalForegroundBoostProcesses(
HWND topLevelWindow,
DWORD processHandleCount,
HANDLE* processHandleArray);

static void SetForegroundProcesses()
{
	static auto _SetAdditionalForegroundBoostProcesses = (decltype(&__SetAdditionalForegroundBoostProcesses))GetProcAddress(GetModuleHandleW(L"user32.dll"), "SetAdditionalForegroundBoostProcesses");

	if (!_SetAdditionalForegroundBoostProcesses)
	{
		return;
	}

	auto window = CoreGetGameWindow();

	if (!window)
	{
		return;
	}

	HANDLE processes[32] = { 0 };
	DWORD numProcesses = 0;

	{
		std::unique_lock _(g_foregroundProcessesMutex);
		for (auto process : g_foregroundProcesses)
		{
			processes[numProcesses++] = process;
		}
	}

	// TEMP: needed as long as this is a LAF: set AppModelFeatureState flag 1 so we pass win32kfull!EditionCanSetAdditionalForegroundBoostProcesses
	uint8_t* peb = (uint8_t*)__readgsqword(0x60);
	peb[0x340] |= 1;

	_SetAdditionalForegroundBoostProcesses(window, numProcesses, processes);
}

static BOOL __stdcall EP_CreateProcessW(const wchar_t* applicationName, wchar_t* commandLine, SECURITY_ATTRIBUTES* processAttributes, SECURITY_ATTRIBUTES* threadAttributes,
										BOOL inheritHandles, DWORD creationFlags, void* environment, const wchar_t* currentDirectory, STARTUPINFOW* startupInfo,
										PROCESS_INFORMATION* information)
{
	// Technically unrelated to this file but it already has a CreateProcessW hook: CEF GPU bits
	bool gpuProcess = false;

	if (wcsstr(commandLine, L"type=gpu"))
	{
		gpuProcess = true;
	}
	// END GPU bits

	// compare the first part of the command line with the Social Club subprocess name
	HMODULE socialClubLib = GetModuleHandle(L"socialclub.dll");

	bool isSubprocess = wcsstr(commandLine, L"subprocess.exe") || StrStrIW(commandLine, L"SocialClubHelper.exe");
	bool isService = wcsstr(commandLine, L"RockstarService.exe");

	if (applicationName || isSubprocess || isService)
	{
		if (isSubprocess ||
			(applicationName && (
				boost::filesystem::path(applicationName).filename() == "subprocess.exe" ||
				boost::filesystem::path(applicationName).filename() == "SocialClubHelper.exe" ||
				boost::filesystem::path(applicationName).filename() == "socialclubhelper.exe")))
		{
			HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, g_rosParentPid);

			information->dwProcessId = g_rosParentPid;
			information->dwThreadId = 0;
			information->hProcess = hProcess;
			information->hThread = INVALID_HANDLE_VALUE;

			SubprocessPipe(commandLine);

			return TRUE;
		}

		if (isService ||
			(applicationName && boost::filesystem::path(applicationName).filename() == "RockstarService.exe"))
		{
			auto mutex = OpenMutexW(SYNCHRONIZE, FALSE, va(L"Cfx_ROSServiceMutex_%s", ToWide(launch::GetLaunchModeKey())));

			if (mutex)
			{
				WaitForSingleObject(mutex, INFINITE);
				CloseHandle(mutex);
			}

			BOOL retval;

			// parse the existing environment block
			EnvironmentMap environmentMap;

			{
				wchar_t* environmentStrings = GetEnvironmentStrings();

				ParseEnvironmentBlock(environmentStrings, environmentMap);

				FreeEnvironmentStrings(environmentStrings);
			}

			// insert tool mode value
			environmentMap[L"CitizenFX_ToolMode"] = L"1";

			// output the environment into a new block
			std::vector<wchar_t> newEnvironment;

			BuildEnvironmentBlock(environmentMap, newEnvironment);

			auto fxApplicationName = MakeCfxSubProcess(L"ROSService", L"game_mtl");

			// set the command line
			const wchar_t* newCommandLine = va(L"\"%s\" ros:service", fxApplicationName);

			// and go create the new fake process
			retval = g_oldCreateProcessW(fxApplicationName, const_cast<wchar_t*>(newCommandLine), processAttributes, threadAttributes, inheritHandles, (creationFlags | CREATE_UNICODE_ENVIRONMENT) & ~CREATE_SUSPENDED, &newEnvironment[0], MakeRelativeCitPath(L"data\\game-storage\\launcher").c_str(), startupInfo, information);

			if (!retval)
			{
				auto error = GetLastError();

				trace("Creating service failed - %d\n", error);
			}
			else
			{
				trace("Got ROS service - pid %d\n", information->dwProcessId);
			}

			static HostSharedData<LauncherState> hsd("CFX_ServicePid");
			hsd->pid = information->dwProcessId;

			SetPriorityClass(information->hProcess, ABOVE_NORMAL_PRIORITY_CLASS);

			information->hThread = NULL;
			information->hProcess = CreateEventW(NULL, TRUE, FALSE, va(L"Cfx_ROSServiceEvent_%s", ToWide(launch::GetLaunchModeKey())));

			return retval;
		}
	}

	if (applicationName)
	{
		if (boost::filesystem::path(applicationName).filename() == L"GTA5.exe" || boost::filesystem::path(applicationName).filename() == L"RDR2.exe")
		{
			g_launchDone = true;

			HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, va(L"CitizenFX_GTA5_ClearedForLaunch%s", IsCL2() ? L"CL2" : L""));

			if (hEvent != INVALID_HANDLE_VALUE)
			{
				SetEvent(hEvent);
				CloseHandle(hEvent);
			}

			HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, g_rosParentPid);

			if (GetFileAttributes(MakeRelativeCitPath(L"permalauncher").c_str()) != INVALID_FILE_ATTRIBUTES)
			{
				AllocConsole();
				SetConsoleTitle(L"GTAVLauncher child - close to terminate");

				CloseHandle(hProcess);

				hProcess = GetCurrentProcess();
			}

			SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);

			// disable D3D present function
			// TODO for MTL
			//hook::return_function(hook::get_pattern("89 74 24 20 44 8D 4E 01 45 33 C0 33 D2 FF", -0x6C));

			std::thread([=] ()
			{
				WaitForSingleObject(hProcess, INFINITE);

				TerminateProcess(GetCurrentProcess(), 0);
			}).detach();

			information->dwProcessId = g_rosParentPid;
			information->dwThreadId = 0;
			information->hProcess = hProcess;
			information->hThread = INVALID_HANDLE_VALUE;

			return TRUE;
		}
	}

	auto rv = g_oldCreateProcessW(applicationName, commandLine, processAttributes, threadAttributes, inheritHandles, creationFlags, environment, currentDirectory, startupInfo, information);

	if (gpuProcess)
	{
		{
			HANDLE newHandle = 0;
			DuplicateHandle(GetCurrentProcess(), information->hProcess, GetCurrentProcess(), &newHandle, 0, FALSE, DUPLICATE_SAME_ACCESS);

			std::unique_lock _(g_foregroundProcessesMutex);
			g_foregroundProcesses.insert(newHandle);
		}

		SetForegroundProcesses();
	}

	return rv;
}

// TODO: factor out
static std::function<void()> g_waitForLauncherCB;

void WaitForLauncher()
{
	if (g_waitForLauncherCB)
	{
		g_waitForLauncherCB();
	}
}

extern void SetCanSafelySkipLauncher(bool value);

extern "C" DLL_EXPORT DWORD WINAPI ROSFailure(LPVOID)
{
	FatalError("Timed out while waiting for the Rockstar Games Launcher (ROS/MTL).\nPlease check your system for third-party software (antivirus, etc.) that might be interfering with the 'embedded' RGL launching the game.\n\nIf asking for support, please save and upload the log file from the 'Save information' button.\n\nAgain, please save and UPLOAD the log file from the 'Save information' button to https://forum.cfx.re/t/2009848 and provide as much information as possible (how often you get this, any installed antivirus or overlays, and so on).");
	return 0;
}

static const void* GetRemoteProcAddress(HANDLE hProcess, const void* function)
{
	HMODULE localModule = nullptr;
	GetModuleHandleExW(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT | GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS, (LPCWSTR)function, &localModule);

	if (localModule)
	{
		uintptr_t rva = reinterpret_cast<uintptr_t>(function) - reinterpret_cast<uintptr_t>(localModule);

		const wchar_t* baseName = nullptr;

		{
			wchar_t moduleName[MAX_PATH * 2];
			GetModuleFileNameW(localModule, moduleName, std::size(moduleName));

			baseName = wcsrchr(moduleName, L'\\') + 1;
		}

		DWORD processLen = 0;
		if (EnumProcessModules(hProcess, nullptr, 0, &processLen))
		{
			std::vector<HMODULE> buffer(processLen / sizeof(HMODULE));

			if (EnumProcessModules(hProcess, buffer.data(), buffer.size() * sizeof(HMODULE), &processLen))
			{
				for (HMODULE module : buffer)
				{
					wchar_t remoteModuleName[MAX_PATH * 2];

					if (GetModuleFileNameExW(hProcess, module, remoteModuleName, std::size(remoteModuleName)))
					{
						const wchar_t* remoteBaseName = wcsrchr(remoteModuleName, L'\\') + 1;

						if (wcsicmp(baseName, remoteBaseName) == 0)
						{
							return reinterpret_cast<const void*>(reinterpret_cast<uintptr_t>(module) + rva);
						}
					}
				}
			}
		}
	}

	return nullptr;
}

void BackOffMtl()
{
	SetCanSafelySkipLauncher(false);

	auto backOffSuffix = fmt::sprintf(L"%d", GetTickCount64());

	auto backOffFile = [backOffSuffix](const std::wstring& fileName)
	{
		MoveFileW(MakeRelativeCitPath(fileName).c_str(), MakeRelativeCitPath(fileName + L".old" + backOffSuffix).c_str());
	};

	backOffFile(L"data\\game-storage\\ros_documents" ROS_SUFFIX_W);
	backOffFile(L"data\\game-storage\\ros_launcher_appdata" ROS_SUFFIX_W);
	backOffFile(L"data\\game-storage\\ros_launcher_data" ROS_SUFFIX_W);
	backOffFile(L"data\\game-storage\\ros_launcher_documents" ROS_SUFFIX_W);
	backOffFile(L"data\\game-storage\\ros_launcher_game" ROS_SUFFIX_W);
	backOffFile(L"data\\game-storage\\ros_profiles");
}

void RunLauncher(const wchar_t* toolName, bool instantWait);
extern bool InLegitimacyUI();

static void SetLauncherWaitCB(HANDLE hEvent, HANDLE hProcessIn, BOOL doBreak, DWORD timeout = INFINITE)
{
	static bool inWait = false;
	static HANDLE hProcess;

	hProcess = hProcessIn;

	if (inWait)
	{
		return;
	}

	g_waitForLauncherCB = [=]()
	{
		inWait = true;

		bool done = false;
		static int retries = 1;
		static uint64_t startTime = GetTickCount64();
		uint64_t endTime = GetTickCount64() + timeout;

		while (!done)
		{
			HANDLE waitHandles[] = { hEvent, hProcess };
			DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, 5000);

			// if we've currently got the LegitimacyNui bits opened, bump the timeout
			if (InLegitimacyUI())
			{
				endTime = GetTickCount64() + timeout;
			}

			if (waitResult == WAIT_OBJECT_0 + 1)
			{
				done = true;

				if (timeout == INFINITE)
				{
					TerminateProcess(GetCurrentProcess(), 0);
				}
			}
			else if (waitResult != WAIT_TIMEOUT)
			{
				done = true;
			}
			else
			{
				if (doBreak)
				{
					auto waitedFor = (GetTickCount64() - startTime) / 1000;

					if (waitedFor >= 45)
					{
						if (retries >= 3)
						{
							BackOffMtl();

							auto threadStart = GetRemoteProcAddress(hProcess, &ROSFailure);

							DWORD tid;
							CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)threadStart, NULL, 0, &tid);

							WaitForSingleObject(GetCurrentProcess(), 30000);

							ROSFailure(nullptr);
							__debugbreak();
						}

						++retries;
						trace("^3Retrying ROS launch (attempt %d/%d)\n", retries, 3);

						HostSharedData<LauncherState> hsd("CFX_ServicePid");

						auto service = OpenProcess(PROCESS_TERMINATE, FALSE, hsd->pid);
						TerminateProcess(service, 0);
						TerminateProcess(hProcess, 0);

						Sleep(1500);

						startTime = GetTickCount64();
						RunLauncher(L"ros:launcher", false);

						continue;
					}

					trace("^3ROS/MTL still hasn't cleared launch (waited %d seconds) - if this ends up timing out, please solve this!\n", waitedFor);
				}

				if (timeout != INFINITE)
				{
					if (GetTickCount64() >= endTime)
					{
						TerminateProcess(hProcess, 0);
						done = true;
					}
				}
			}
		}

		inWait = false;
	};
}

void RunLauncherAwait()
{
	if (g_waitForLauncherCB)
	{
		return;
	}

	static HostSharedData<LauncherState> launcherState("CfxLauncherState");

	HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, va(L"CitizenFX_GTA5_ClearedForLaunch%s", IsCL2() ? L"CL2" : L""));

	while (launcherState->pid == 0)
	{
		Sleep(1000);

		if (launcherState->pid == 0)
		{
			trace("Haven't got launcher PID yet...\n");
		}
	}

	HANDLE hProcess = OpenProcess(SYNCHRONIZE | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, launcherState->pid);

	SetLauncherWaitCB(hEvent, hProcess, TRUE);
}

void RunLauncher(const wchar_t* toolName, bool instantWait)
{
	// early out if the launcher pipe already exists
	if (WaitNamedPipe(va(L"%s%s", PIPE_NAME, IsCL2() ? L"_CL2" : L""), 15) != 0)
	{
		trace("Already found a " PIPE_NAME_NARROW ", not starting child launcher.\n");

		return;
	}

	static HostSharedData<LauncherState> launcherState("CfxLauncherState");

	// parse the existing environment block
	EnvironmentMap environmentMap;

	{
		wchar_t* environmentStrings = GetEnvironmentStrings();

		ParseEnvironmentBlock(environmentStrings, environmentMap);

		FreeEnvironmentStrings(environmentStrings);
	}

	// insert tool mode value
	environmentMap[L"CitizenFX_ToolMode"] = L"1";

	// output the environment into a new block
	std::vector<wchar_t> newEnvironment;

	BuildEnvironmentBlock(environmentMap, newEnvironment);

	// not as safe as recreating the environment block, but easier
	//SetEnvironmentVariable(L"CitizenFX_ToolMode", L"1");

	// create a new application name
	auto fxApplicationName = MakeCfxSubProcess(L"ROSLauncher", L"game_mtl");

	// set the command line
	//const wchar_t* newCommandLine = va(L"\"%s\" %s --parent_pid=%d \"%s\"", fxApplicationName, toolName, GetCurrentProcessId(), MakeRelativeGamePath(L"GTAVLauncher.exe").c_str());
	const wchar_t* newCommandLine = va(L"\"%s\" %s --parent_pid=%d \"%s\" -noRecogniser", fxApplicationName, toolName, GetCurrentProcessId(), L"C:\\program files\\rockstar games\\launcher\\launcher.exe");

	// create a waiting event
	auto eventName = L"";

	if (wcsstr(toolName, L"steam"))
	{
		eventName = L"_Steam";
	}
	else if (wcsstr(toolName, L"epic"))
	{
		eventName = L"_Epic";
	}
	else if (IsCL2())
	{
		eventName = L"CL2";
	}

	HANDLE hEvent = CreateEvent(nullptr, TRUE, FALSE, va(L"CitizenFX_GTA5_ClearedForLaunch%s", eventName));

	// and go create the new fake process
	auto isLauncher = (wcsstr(toolName, L"ros:launcher") != nullptr);

	PROCESS_INFORMATION pi;
	STARTUPINFO si = { sizeof(STARTUPINFO) };
	BOOL retval = g_oldCreateProcessW(fxApplicationName, const_cast<wchar_t*>(newCommandLine), nullptr, nullptr, FALSE, CREATE_UNICODE_ENVIRONMENT, &newEnvironment[0], MakeRelativeCitPath(L"data\\game-storage\\launcher").c_str(), &si, &pi);

	if (!retval)
	{
		auto error = GetLastError();

		trace("Creating %s process failed - %d\n", ToNarrow(toolName), error);
	}
	else
	{
		trace("Got %s process - pid %d\n", ToNarrow(toolName), pi.dwProcessId);

		if (isLauncher)
		{
			launcherState->pid = pi.dwProcessId;
		}

		DWORD timeout = INFINITE;

		if (wcsstr(toolName, L"ros:epic") != nullptr || wcsstr(toolName, L"ros:steam") != nullptr)
		{
			timeout = 30000;
		}

		SetPriorityClass(pi.hProcess, ABOVE_NORMAL_PRIORITY_CLASS);

		SetLauncherWaitCB(hEvent, pi.hProcess, isLauncher, timeout);

        if (instantWait)
        {
            g_waitForLauncherCB();
			ResetEvent(hEvent);
        }
	}
}

static DWORD(__stdcall* g_oldGetModuleFileNameW)(HMODULE, LPWSTR, DWORD);

DWORD ToolGetModuleFileNameW(LPWSTR fileName, DWORD nSize);

extern std::wstring g_origProcess;

static DWORD __stdcall EP_GetModuleFileNameW(HMODULE module, LPWSTR fileName, DWORD nSize)
{
	if (module == nullptr && !g_origProcess.empty())
	{
		if (getenv("CitizenFX_ToolMode"))
		{
			return ToolGetModuleFileNameW(fileName, nSize);
		}
	}

	return g_oldGetModuleFileNameW(module, fileName, nSize);
}

#include <wininet.h>

#pragma comment(lib, "wininet.lib")

static BOOL(WINAPI* g_oldInternetGetConnectedState)(LPDWORD lpdwFlags, DWORD dwReserved);

static BOOL WINAPI EP_InternetGetConnectedState(LPDWORD lpdwFlags, DWORD dwReserved)
{
	return TRUE;
}

static HINTERNET(__stdcall* g_oldInternetOpenW)(LPCTSTR lpszAgent, DWORD dwAccessType, LPCTSTR lpszProxyName, LPCTSTR lpszProxyBypass, DWORD dwFlags);

static HINTERNET __stdcall EP_InternetOpenW(LPCTSTR lpszAgent, DWORD dwAccessType, LPCTSTR lpszProxyName, LPCTSTR lpszProxyBypass, DWORD dwFlags)
{
	HINTERNET theInternet = g_oldInternetOpenW(lpszAgent, INTERNET_OPEN_TYPE_DIRECT, lpszProxyName, lpszProxyBypass, dwFlags);

	INTERNET_PROXY_INFO proxyInfo;
	proxyInfo.dwAccessType = INTERNET_OPEN_TYPE_DIRECT;
	proxyInfo.lpszProxy = L"";
	proxyInfo.lpszProxyBypass = L"";
	
	InternetSetOption(theInternet, INTERNET_OPTION_PROXY, &proxyInfo, sizeof(proxyInfo));

	return theInternet;
}

static BOOL(__stdcall* g_oldHttpQueryInfoW)(HANDLE internet, DWORD infoLevel, LPVOID buffer, LPDWORD bufferLength, LPDWORD index);

static BOOL __stdcall EP_HttpQueryInfoW(HANDLE internet, DWORD infoLevel, LPVOID buffer, LPDWORD bufferLength, LPDWORD index)
{
	if (infoLevel == (HTTP_QUERY_DATE | HTTP_QUERY_FLAG_SYSTEMTIME))
	{
		if (*bufferLength < sizeof(SYSTEMTIME))
		{
			*bufferLength = sizeof(SYSTEMTIME);

			SetLastError(ERROR_INSUFFICIENT_BUFFER);

			return FALSE;
		}

		SYSTEMTIME systemTime;
		GetSystemTime(&systemTime);

		*bufferLength = sizeof(SYSTEMTIME);
		*(SYSTEMTIME*)buffer = systemTime;

		return TRUE;
	}

	return g_oldHttpQueryInfoW(internet, infoLevel, buffer, bufferLength, index);
}

static HANDLE(__stdcall* g_oldCreateFileA)(LPCSTR, DWORD, DWORD, LPSECURITY_ATTRIBUTES, DWORD, DWORD, HANDLE);

static hook::cdecl_stub<void(int, int)> setupLoadingScreens([]()
{
#if defined(GTA_FIVE)
	return hook::get_call(hook::get_pattern("8D 4F 08 33 D2 E8 ? ? ? ? C6", 5));
#else
	return (void*)0;
#endif
});

static HANDLE __stdcall CreateFileAStub(
	_In_     LPCSTR               lpFileName,
	_In_     DWORD                 dwDesiredAccess,
	_In_     DWORD                 dwShareMode,
	_In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes,
	_In_     DWORD                 dwCreationDisposition,
	_In_     DWORD                 dwFlagsAndAttributes,
	_In_opt_ HANDLE                hTemplateFile)
{
	if (strcmp(lpFileName, PIPE_NAME_NARROW) == 0)
	{
		trace("^2Opening %s, waiting for launcher to load...\n", PIPE_NAME_NARROW);

		lpFileName = va("%s%s", lpFileName, IsCL2() ? "_CL2" : "");

		WaitForLauncher();

		trace("^2Launcher gave all-clear - waiting for pipe.\n");

		WaitNamedPipeA(lpFileName, NMPWAIT_WAIT_FOREVER);

		trace("^2Launcher is fine, continuing to initialize!\n");
	}
	else if (strcmp(lpFileName, "\\\\.\\pipe\\MTLService_Pipe") == 0)
	{
		lpFileName = va("\\\\.\\pipe\\MTLService_Pipe_CFX%s", IsCL2() ? "_CL2" : "");
	}
	else if (strcmp(lpFileName, "\\\\.\\pipe\\MTLLauncher_Pipe") == 0)
	{
		lpFileName = va("\\\\.\\pipe\\MTLLauncher_Pipe_CFX%s", IsCL2() ? "_CL2" : "");
	}
	
	return g_oldCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

HANDLE _stdcall CreateNamedPipeAHookL(_In_ LPCSTR lpName, _In_ DWORD dwOpenMode, _In_ DWORD dwPipeMode, _In_ DWORD nMaxInstances, _In_ DWORD nOutBufferSize, _In_ DWORD nInBufferSize, _In_ DWORD nDefaultTimeOut, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
	if (strcmp(lpName, PIPE_NAME_NARROW) == 0)
	{
		lpName = va("%s%s", lpName, IsCL2() ? "_CL2" : "");
	}
	else if (strstr(lpName, "MTLLauncher"))
	{
		lpName = va("\\\\.\\pipe\\MTLLauncher_Pipe_CFX%s", IsCL2() ? "_CL2" : "");
	}

	return CreateNamedPipeA(lpName, dwOpenMode, dwPipeMode, nMaxInstances, nOutBufferSize, nInBufferSize, nDefaultTimeOut, lpSecurityAttributes);
}

static HANDLE(__stdcall *g_oldOpenFileMappingA)(_In_ DWORD dwDesiredAccess, _In_ BOOL bInheritHandle, _In_ LPCSTR lpName);

static HANDLE __stdcall OpenFileMappingAStub(_In_ DWORD dwDesiredAccess, _In_ BOOL bInheritHandle, _In_ LPCSTR lpName)
{
	if (lpName && strstr(lpName, "MTLService_FileMapping"))
	{
		lpName = va("MTLService_FileMapping_CFX%s", IsCL2() ? "_CL2" : "");
	}

	return g_oldOpenFileMappingA(dwDesiredAccess, bInheritHandle, lpName);
}

static BOOL(__stdcall *g_oldWriteFile)(_In_ HANDLE hFile, _In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ DWORD nNumberOfBytesToWrite, _Out_opt_ LPDWORD lpNumberOfBytesWritten, _Inout_opt_ LPOVERLAPPED lpOverlapped);

static BOOL __stdcall WriteFileStub(_In_ HANDLE hFile, _In_reads_bytes_opt_(nNumberOfBytesToWrite) LPCVOID lpBuffer, _In_ DWORD nNumberOfBytesToWrite, _Out_opt_ LPDWORD lpNumberOfBytesWritten, _Inout_opt_ LPOVERLAPPED lpOverlapped)
{
	return g_oldWriteFile(hFile, lpBuffer, nNumberOfBytesToWrite, lpNumberOfBytesWritten, lpOverlapped);
}

#include <MinHook.h>

void OnPreInitHook()
{
	static bool did = false;

	if (did)
	{
		return;
	}

	did = true;

	g_manager = new LoopbackTcpServerManager();
	Instance<LoopbackTcpServerManager>::Set(g_manager);

	if (getenv("CitizenFX_ToolMode"))
	{
		wchar_t computerName[512];
		DWORD len = std::size(computerName);
		GetComputerNameW(computerName, &len);

		static auto localServer = g_manager->RegisterTcpServer(ToNarrow(computerName));
	}

	MH_Initialize();

	LoadLibrary(L"wininet.dll");

#define DO_HOOK(dll, func, ep, oldEP) \
	{ auto err = MH_CreateHookApi(dll, func, ep, reinterpret_cast<void**>(&oldEP)); assert(err == MH_OK); }

#define DO_HOOK_NO_ASSERT(dll, func, ep, oldEP) \
	{ MH_CreateHookApi(dll, func, ep, reinterpret_cast<void**>(&oldEP)); }
	
 	DO_HOOK(L"ws2_32.dll", "closesocket", EP_CloseSocket, g_oldCloseSocket);
  	DO_HOOK(L"ws2_32.dll", "connect", EP_Connect, g_oldConnect);
  	DO_HOOK(L"ws2_32.dll", "recv", EP_Recv, g_oldRecv);
  	DO_HOOK(L"ws2_32.dll", "select", EP_Select, g_oldSelect);
  	DO_HOOK(L"ws2_32.dll", "send", EP_Send, g_oldSend);
  	DO_HOOK(L"ws2_32.dll", "gethostbyname", EP_GetHostByName, g_oldGetHostByName);
	DO_HOOK(L"ws2_32.dll", "getsockname", EP_GetSockName, g_oldGetSockName);
	DO_HOOK(L"ws2_32.dll", "getpeername", EP_GetPeerName, g_oldGetPeerName);
	DO_HOOK(L"ws2_32.dll", "setsockopt", EP_SetSockOpt, g_oldSetSockOpt);
	DO_HOOK(L"ws2_32.dll", "gethostname", EP_GetHostName, g_oldGetHostName);
	DO_HOOK(L"ws2_32.dll", "GetAddrInfoW", EP_GetAddrInfoW, g_oldGetAddrInfoW);
	DO_HOOK(L"ws2_32.dll", "GetAddrInfoExW", EP_GetAddrInfoExW, g_oldGetAddrInfoExW);
//	DO_HOOK(L"ws2_32.dll", "FreeAddrInfoW", EP_FreeAddrInfoW, g_oldFreeAddrInfoW); // these three are hook-checked

#ifndef _M_IX86
	if (CfxIsWine() || GetModuleHandle(L"xtajit64.dll") != nullptr) // xtajit64.dll = arm64 jit
	{
		DO_HOOK(L"ws2_32.dll", "getaddrinfo", EP_GetAddrInfo, g_oldGetAddrInfo); // enabled because wine, probably going to fail
	}
#endif

//	DO_HOOK(L"ws2_32.dll", "freeaddrinfo", EP_FreeAddrInfo, g_oldFreeAddrInfo);
 	DO_HOOK(L"ws2_32.dll", "WSALookupServiceBeginW", EP_WSALookupServiceBeginW, g_oldLookupService);
	DO_HOOK(L"ws2_32.dll", "WSAEventSelect", EP_WSAEventSelect, g_oldEventSelect);
	DO_HOOK(L"ws2_32.dll", "WSAEnumNetworkEvents", EP_WSAEnumNetworkEvents, g_oldEnumEvents);
	DO_HOOK(L"ws2_32.dll", "WSASend", EP_WSASend, g_oldWSASend);
	DO_HOOK(L"ws2_32.dll", "WSARecv", EP_WSARecv, g_oldWSARecv);
	DO_HOOK(L"ws2_32.dll", "WSAIoctl", EP_WSAIoctl, g_oldWSAIoctl);

	// as it's used from inside Windows components, we need to hook kernelbase - MinWin and all...
	// -- and of course, wininet is still an IE component officially prior to Win10, so it doesn't use MinWin imports too much...
	// ... let alone that the thread pool is still not in MinWin in 7 to 8.1, or even Vista not having MinWin in the first place
	// ...... and scratch the above, it's in *both* DLLs. what the literal fuck MSFT?!

	{
		DO_HOOK(L"kernelbase.dll", "CreateThreadpoolIo", EP_CreateThreadpoolIo, g_oldCreateThreadpoolIo);
		DO_HOOK(L"kernelbase.dll", "CallbackMayRunLong", EP_CallbackMayRunLong, g_oldCallbackMayRunLong);

		DO_HOOK_NO_ASSERT(L"kernel32.dll", "CreateThreadpoolIo", EP_CreateThreadpoolIo, g_oldCreateThreadpoolIo);
		DO_HOOK_NO_ASSERT(L"kernel32.dll", "CallbackMayRunLong", EP_CallbackMayRunLong, g_oldCallbackMayRunLong);
	}

	if (!CfxIsWine())
	{
		DO_HOOK(L"kernel32.dll", "CreateProcessW", EP_CreateProcessW, g_oldCreateProcessW);
	}
	else
	{
		DO_HOOK(L"kernelbase.dll", "CreateProcessW", EP_CreateProcessW, g_oldCreateProcessW);
	}

	if (getenv("CitizenFX_ToolMode") && !strstr(GetCommandLineA(), "ros:launcher")) // hacky...
	{
		DO_HOOK(L"kernel32.dll", "GetModuleFileNameW", EP_GetModuleFileNameW, g_oldGetModuleFileNameW);
	}

	DO_HOOK(L"wininet.dll", "HttpQueryInfoW", EP_HttpQueryInfoW, g_oldHttpQueryInfoW);
	DO_HOOK(L"wininet.dll", "InternetOpenW", EP_InternetOpenW, g_oldInternetOpenW);
	DO_HOOK(L"wininet.dll", "InternetGetConnectedState", EP_InternetGetConnectedState, g_oldInternetGetConnectedState);

	DO_HOOK(L"kernel32.dll", "CreateFileA", CreateFileAStub, g_oldCreateFileA);
	DO_HOOK(L"kernel32.dll", "OpenFileMappingA", OpenFileMappingAStub, g_oldOpenFileMappingA);
	DO_HOOK(L"kernel32.dll", "WriteFile", WriteFileStub, g_oldWriteFile);

	trace("hello from %s\n", GetCommandLineA());

	MH_EnableHook(MH_ALL_HOOKS);
}

static InitFunction lateInitFunction([]()
{
	OnPreInitHook();
}, -1000);

static void(*g_origRlineInit)(int);
static void(*g_origCloudInit)(int);
static void(*g_origTextureInit)();

static bool InitRlineWrap()
{
	g_origRlineInit(1);
	g_origCloudInit(1);
	g_origTextureInit();

	return true;
}

template<typename TFn, typename T>
inline void NopAndSetCall(TFn* fn, T loc)
{
	hook::set_call(fn, loc);
	hook::nop(loc, 5);
}

static HookFunction hookFunction2([]()
{
#ifdef GTA_FIVE
	auto loc = hook::get_pattern<char>("33 C9 E8 ? ? ? ? 41 8B CE E8 ? ? ? ? 41", 10);
	NopAndSetCall(&g_origRlineInit, loc);
	NopAndSetCall(&g_origCloudInit, loc + 8);
	NopAndSetCall(&g_origTextureInit, hook::get_pattern("C7 45 38 99 3B 6D F6", 19));

	hook::call(hook::get_pattern("8D 4A 03 E8 ? ? ? ? E8 ? ? ? ? 84 C0 75", 8), InitRlineWrap);
#endif
});
