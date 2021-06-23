#include <StdInc.h>
#include <MinHook.h>
#include <Hooking.h>

#include <WS2tcpip.h>

#include <CoreConsole.h>
#include <CoreNetworking.h>
#include <NetLibrary.h>

#include <nutsnbolts.h>

#include <botan/base64.h>

#pragma comment(lib, "ws2_32.lib")

NetLibrary* g_netLibrary;
static SOCKET g_gameSocket;

int __stdcall CfxSendTo(SOCKET s, char* buf, int len, int flags, sockaddr* to, int tolen)
{
	sockaddr_in* toIn = (sockaddr_in*)to;

	if (s == g_gameSocket)
	{
		if (toIn->sin_addr.S_un.S_un_b.s_b1 == 0xC0 && toIn->sin_addr.S_un.S_un_b.s_b2 == 0xA8)
		{
			//if (CoreIsDebuggerPresent())
			{
				//trace("CfxSendTo (to internal address %i) %i b (from thread 0x%x)\n", (htonl(toIn->sin_addr.s_addr) & 0xFFFF) ^ 0xFEED, len, GetCurrentThreadId());
			}
		}
		else
		{
			char publicAddr[256];
			inet_ntop(AF_INET, &toIn->sin_addr.s_addr, publicAddr, sizeof(publicAddr));

			if (toIn->sin_addr.s_addr == 0xFFFFFFFF)
			{
				return len;
			}

			//trace("CfxSendTo (to %s) %i b\n", publicAddr, len);
		}

		g_netLibrary->RoutePacket(buf, len, (uint16_t)((htonl(toIn->sin_addr.s_addr)) & 0xFFFF) ^ 0xFEED);

		return len;
	}

	return sendto(s, buf, len, flags, to, tolen);
}

int __stdcall CfxRecvFrom(SOCKET s, char* buf, int len, int flags, sockaddr* from, int* fromlen)
{
	static char buffer[65536];
	size_t length;
	uint16_t netID;

	if (s == g_gameSocket)
	{
		if (g_netLibrary->DequeueRoutedPacket(buffer, &length, &netID))
		{
			memcpy(buf, buffer, fwMin((size_t)len, length));

			sockaddr_in* outFrom = (sockaddr_in*)from;
			memset(outFrom, 0, sizeof(sockaddr_in));

			outFrom->sin_family = AF_INET;
			outFrom->sin_addr.s_addr = ntohl((netID ^ 0xFEED) | 0xC0A80000);
			outFrom->sin_port = htons(6672);

			char addr[60];
			inet_ntop(AF_INET, &outFrom->sin_addr.s_addr, addr, sizeof(addr));

			*fromlen = sizeof(sockaddr_in);

			//if (CoreIsDebuggerPresent())
			{
				//trace("CfxRecvFrom (from %i %s) %i bytes on %p\n", netID, addr, length, (void*)s);
			}

			return fwMin((size_t)len, length);
		}
		else
		{
			WSASetLastError(WSAEWOULDBLOCK);
			return SOCKET_ERROR;
		}
	}

	return recvfrom(s, buf, len, flags, from, fromlen);
}

int __stdcall CfxBind(SOCKET s, sockaddr* addr, int addrlen)
{
	sockaddr_in* addrIn = (sockaddr_in*)addr;

	if (htons(addrIn->sin_port) == 6672)
	{
		if (wcsstr(GetCommandLine(), L"cl2"))
		{
			addrIn->sin_port = htons(6673);
		}

		g_gameSocket = s;
	}

	return bind(s, addr, addrlen);
}

namespace rage
{
	struct netSocket
	{
		size_t a;
		SOCKET sock;
	};

	struct netRelayEventPacketReceived
	{
		void* fakeVtbl;
		uint8_t pad[28];
		int type; // 2;
		char addressBuf[16];
		void* data;
		size_t len;
	};

	struct netSocketEventReceiveThreadTicked
	{
		void* fakeVtbl;
		uint8_t pad[28];
		int type; // 4;
	};
}

void(*g_origSocketInit)();
static rage::netSocket* socketPtr;

bool(__cdecl* g_origGetLocalPeerAddress)(rage::netPeerAddress* out);

int(_fastcall* g_origParsePacket)(int*, void*);

int __fastcall ParsePacketHook(int* a1, void* a2)
{
	auto result = g_origParsePacket(a1, a2);

	if (*a1 == -1)
	{
		trace("-1?\n");
	}

	trace(__FUNCTION__ " packet: %i\n", *a1);
	return result;
}

static bool __stdcall ReadSession(void* parTree, rage::rlSessionInfo* session)
{
	if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
	{
		return false;
	}

	session->sessionToken.token1 = 2;
	session->sessionToken.token2 = 2;

	auto address = &session->peerAddress;
	memset(address, 0, sizeof(*address));

	address->secKeyTime = g_netLibrary->GetHostBase() ^ 0xABCD;
	address->unkKey1 = g_netLibrary->GetHostBase();
	address->unkKey2 = g_netLibrary->GetHostBase();
	address->localAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
	address->localAddr.port = 6672;
	address->relayAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
	address->relayAddr.port = 6672;
	address->publicAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
	address->publicAddr.port = 6672;
	address->rockstarAccountId = g_netLibrary->GetHostNetID();

	uint8_t sessionBlob[69];
	size_t l = 0;
	// .43
	//((void(__thiscall*)(rage::rlSessionInfo*, uint8_t*, size_t, size_t*))0x6C8560)(session, sessionBlob, 69, &l);

	//trace("tryna join %s\n", Botan::base64_encode(sessionBlob, l));

	return true;
}

struct OnlineAddress
{
	uint32_t ip1;
	uint16_t port1;
	uint32_t ip2;
	uint16_t port2;
};

static int g_localAddress = getenv("COMPUTERNAME")[0] == 'F' ? 1 : 2;

bool __cdecl GetLocalPeerAddressHook(rage::netPeerAddress* address)
{
	auto success = g_origGetLocalPeerAddress(address);

	/*if (getenv("ROS_TEMP_LAN_IP"))
	{
		auto a = htonl(inet_addr(getenv("ROS_TEMP_LAN_IP")));;
		auto b = 6672;

		if (wcsstr(GetCommandLine(), L"cl2"))
		{
			a = htonl(inet_addr("127.0.0.1"));
			b = 6673;
		}

		out->lanIP = a;
		out->onlineIP = a;
		out->unkIP = a;
		out->lanPort = b;
		out->onlinePort = b;
		out->unkPort = b;
	}

	//out->unkIP = GetTickCount(); // kinda shitty but irrelevant later

	return success;*/

	// .43
	* (uint8_t*)0x18B82CC = 1;
	*(uint32_t*)0x18B82D0 = g_netLibrary->GetServerBase() ^ 0xABCD;
	*(uint64_t*)0x1BB3970 = g_netLibrary->GetServerBase();
	*(uint64_t*)(0x1BB3970 + 8) = g_netLibrary->GetServerBase();
	//*(uint64_t*)0x19F3278 = g_netLibrary->GetServerNetID();

	static auto onlineAddress = *hook::get_pattern<OnlineAddress*>("50 53 8D 44 24 24 50 68 ? ? ? ? 8D", 8);
	onlineAddress->ip1 = onlineAddress->ip2 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	onlineAddress->port1 = onlineAddress->port2 = 6672;

	// .43
	static auto onlineAddress2 = (OnlineAddress*)0x11103A8;
	onlineAddress2->ip1 = onlineAddress2->ip2 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	onlineAddress2->port1 = onlineAddress2->port2 = 6672;

	//memset(address, 0, sizeof(*address));

	address->secKeyTime = g_netLibrary->GetServerBase() ^ 0xABCD;
	address->unkKey1 = g_netLibrary->GetServerBase();
	address->unkKey2 = g_netLibrary->GetServerBase();
 	address->localAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
 	address->localAddr.port = 6672;
 	address->relayAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
 	address->relayAddr.port = 6672;
 	address->publicAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
 	address->publicAddr.port = 6672;
	/*address->localAddr.ip.addr = (g_localAddress ^ 0xFEED) | 0xc0a80000;
	address->localAddr.port = 6672;
	address->relayAddr.ip.addr = (g_localAddress ^ 0xFEED) | 0xc0a80000;
	address->relayAddr.port = 6672;
	address->publicAddr.ip.addr = (g_localAddress ^ 0xFEED) | 0xc0a80000;
	address->publicAddr.port = 6672;*/
	address->rockstarAccountId = g_netLibrary->GetServerNetID();

	return true;
}

void SocketInitHook()
{
	socketPtr = *hook::get_pattern<rage::netSocket*>("8D 44 24 0C 50 56 FF 75 0C", 10);
	((void(__fastcall*)(void*, uint16_t))hook::get_pattern("F6 86 A0 00 00 00 20 75 3A", -0x29))(socketPtr, 6672); // bind wrap

	// yes, the relay is active
	**hook::get_pattern<char*>("80 3D ? ? ? ? 00 74 32 6A 00 68", 2) = 1;

	static auto onRelayReceived = ((void(__thiscall*)(void*, void*))hook::get_pattern("8B 44 24 04 83 78 20 02"));
	static auto onSocketEventReceiveThreadTicked = ((void(__thiscall*)(void*, void*))hook::get_pattern("83 78 20 04 8B F1", -5));
	static auto receiveWrap = ((int(__thiscall*)(rage::netSocket*, void* address, void* buf, size_t len, int* outErr))hook::get_pattern("83 EC 1C 53 55 8B E9 56 83 7D 04"));

	g_netLibrary->OnConnectOKReceived.Connect([](NetAddress sv)
	{
		static auto onlineAddress = *hook::get_pattern<OnlineAddress*>("50 53 8D 44 24 24 50 68 ? ? ? ? 8D", 8);
		onlineAddress->ip1 = onlineAddress->ip2 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
		onlineAddress->port1 = onlineAddress->port2 = 6672;
	});

	OnGameFrame.Connect([]()
	{
		static auto connectionMgrPtr = *hook::get_pattern<void*>("8b 01 6A 07 68", 5);

		uint8_t from[8]; // actually: netSocketAddress
		uint8_t buf[1400];

		int rlen = 0;

		do
		{
			rlen = receiveWrap(
				socketPtr,
				&from,
				buf,
				sizeof(buf),
				NULL
				); // receive wrap

			if (rlen > 0)
			{
				//trace(__FUNCTION__ " got connection %d\n", rlen);

				rage::netRelayEventPacketReceived event;
				event.type = 2;
				memset(event.addressBuf, 0, 16);
				*(uint32_t*)&event.addressBuf[0] = 0xFFFFFFFF; // relay
				memcpy(&event.addressBuf[8], from, 8); // direct
				event.data = buf;
				event.len = rlen;

				onRelayReceived(connectionMgrPtr, &event);
			}
		} while (rlen > 0);

		{
			rage::netSocketEventReceiveThreadTicked event;
			event.type = 4;

			onSocketEventReceiveThreadTicked(connectionMgrPtr, &event);
		}
	});

	g_origSocketInit();
}

static hook::cdecl_stub<bool()> isNetworkHost([]()
{
	return hook::get_call(*hook::get_pattern<void*>("68 00 16 5E 2E", -4));
});

static void __fastcall HashSecKeyAddress(uint64_t* outValue, uint32_t seed)
{
	outValue[0] = seed ^ 0xABCD;
	outValue[1] = seed ^ 0xABCD;
}

static bool __fastcall GetOurSystemKey(char* systemKey)
{
	memset(systemKey, 0, 8);
	*(uint32_t*)&systemKey[0] = g_netLibrary->GetServerBase() ^ 0xFEAFEDE;

	return true;
}

static bool __fastcall GetForcedUuid(char* sessionId)
{
	*(uint64_t*)sessionId = 2;

	return true;
}

static bool* didPresenceStuff;

static hook::cdecl_stub<void()> _doPresenceStuff([]()
{
	// .43
	return (void*)0x6C3E60;
});

static void (*g_origSetFilterMenuOn)(void*);

static void SetFilterMenuOnHook(void* a1)
{
	*didPresenceStuff = false;
	_doPresenceStuff();

	g_origSetFilterMenuOn(a1);
}

static uintptr_t g_firstSessionTransactor;
static void (*g_origShutdownCall)();

static hook::thiscall_stub<void(uintptr_t)> netTransactor_Reset([]()
{
	return hook::get_pattern("83 3E 00 74 5A 83 7E 0C 00", -0xC);
});

static void ShutdownCallWrap()
{
	g_origShutdownCall();

	for (int i = 0; i < 16; i++)
	{
		auto transactor = g_firstSessionTransactor + (i * 7 * 4);
		netTransactor_Reset(transactor);
	}
}

static std::string g_quitMsg;

static void WINAPI ExitProcessReplacement(UINT exitCode)
{
	if (g_netLibrary)
	{
		g_netLibrary->Disconnect((g_quitMsg.empty()) ? "Exiting" : g_quitMsg.c_str());
	}

	TerminateProcess(GetCurrentProcess(), exitCode);
}

static void(__thiscall *g_origMigrateCopy)(void*, void*);

void* __fastcall MigrateSessionCopy(char* target, void* /* dummy */, char* source)
{
	g_origMigrateCopy(target, source);

	auto sessionAddress = reinterpret_cast<rage::rlSessionInfo*>(target);

	std::unique_ptr<net::Buffer> msgBuffer(new net::Buffer(64));

	msgBuffer->Write<uint32_t>((sessionAddress->peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED);
	msgBuffer->Write<uint32_t>(sessionAddress->peerAddress.unkKey1);

	g_netLibrary->SendReliableCommand("msgHeHost", reinterpret_cast<const char*>(msgBuffer->GetBuffer()), msgBuffer->GetCurOffset());

	return target;
}

static HookFunction hookFunction([]()
{
	didPresenceStuff = *hook::get_pattern<bool*>("53 56 84 D2", 12);

	static ConsoleCommand quitCommand("quit", [](const std::string& message)
	{
		g_quitMsg = message;
		ExitProcess(-1);
	});

	g_netLibrary = NetLibrary::Create();
	Instance<NetLibrary>::Set(g_netLibrary);

	g_netLibrary->SetBase(GetTickCount());

	g_netLibrary->OnBuildMessage.Connect([](const std::function<void(uint32_t, const char*, int)>& writeReliable)
	{
		static bool lastHostState;

		// hostie
		bool isHost = isNetworkHost();
		if (isHost != lastHostState)
		{
			if (isHost)
			{
				auto base = g_netLibrary->GetServerBase();
				writeReliable(HashRageString("msgIHost"), (char*)&base, sizeof(base));
			}

			lastHostState = isHost;
		}

		static uint32_t lastHostSend = GetTickCount();

		if ((GetTickCount() - lastHostSend) > 1500)
		{
			lastHostState = false;
			lastHostSend = GetTickCount();
		}
	});

	OnCriticalGameFrame.Connect([]()
	{
		g_netLibrary->RunFrame();
	});

	OnMainGameFrame.Connect([]()
	{
		g_netLibrary->RunMainFrame();
	});

	hook::iat("ws2_32.dll", CfxBind, 2);
	hook::iat("ws2_32.dll", CfxSendTo, 20);
	hook::iat("ws2_32.dll", CfxRecvFrom, 17);

	MH_Initialize();

	// exitprocess -> terminateprocess
	MH_CreateHookApi(L"kernel32.dll", "ExitProcess", ExitProcessReplacement, nullptr);

	// SetFilterMenuOn
	MH_CreateHook(*hook::get_pattern<void*>("68 ? ? ? ? 68 49 36 F4 18", 1), SetFilterMenuOnHook, (void**)&g_origSetFilterMenuOn);

	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? B9 16 00 00 00")), SocketInitHook, (void**)&g_origSocketInit);
	{
		auto location = hook::get_pattern<char>("55 8B EC 83 E4 F8 83 EC 18 33 C0");
		MH_CreateHook(location, GetLocalPeerAddressHook, (void**)&g_origGetLocalPeerAddress);

		// used to derive session key from session key seed in session offer
		hook::jump(hook::get_call(location + 0xA0), HashSecKeyAddress); // rage::netCrypto::GenerateRandomBytes(?)
	}

	//MH_CreateHook(hook::get_pattern("51 53 56 8B F2 32 DB"), ParsePacketHook, (void**)&g_origParsePacket);
	MH_EnableHook(MH_ALL_HOOKS);

	// unset session transactor before shutting down connection manager
	{
		auto location = hook::get_pattern("84 DB 0F 84 ? 00 00 00 E8 ? ? ? ? B9 ? ? ? ? E8", 8);
		hook::set_call(&g_origShutdownCall, location);
		hook::call(location, ShutdownCallWrap);
	}

	g_firstSessionTransactor = *hook::get_pattern<uintptr_t>("75 51 33 DB BE", -5);

	// temp check: SendConnectionless connection check for deferred Pack() queue
	//hook::put<uint16_t>(0x6CF1D5, 0xE990);

	//hook::jump(hook::get_pattern("83 F8 FF 75 20 8D 4C 24 08", -0x29), GetOurSystemKey);
	//hook::jump(hook::get_call(hook::get_pattern("84 DB 74 17 8D 4C 24 1C E8", 8)), GetOurSystemKey);
	//hook::jump(hook::get_call(hook::get_pattern("84 DB 74 17 8D 4C 24 1C E8", 8)), GetForcedUuid); // too much
	hook::call(hook::get_pattern("84 DB 74 17 8D 4C 24 1C E8", 8), GetForcedUuid); // peer ID (joinee)
	hook::call(hook::get_pattern("8D 4C 24 08 66 0F 13 44 24 10 E8", 10), GetForcedUuid); // peer ID (migrate)
	hook::call(hook::get_pattern("10 E8 ? ? ? ? 84 C0 74 5E 8B", 1), GetForcedUuid); // peer ID (host)
	hook::jump(hook::get_pattern("74 36 83 7E 24 00 74 30", -0x48), ReadSession);

	void* migrateCmd = hook::get_pattern("8D 47 60 50 81 C1 88 00 00 00", 10);
	hook::set_call(&g_origMigrateCopy, migrateCmd);
	hook::call(migrateCmd, (void*)&MigrateSessionCopy);
});
