#include <StdInc.h>

#include <Hooking.h>
#include <NetLibrary.h>

#include <GameInit.h>

#include <CoreNetworking.h>

NetLibrary* g_netLibrary;

// shared relay functions (from early rev. gta:net:five; do update!)
#include <ws2tcpip.h>

static SOCKET g_gameSocket;
static int lastReceivedFrom;

static ULONG g_pendSendVar;

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

			lastReceivedFrom = netID;

			if (CoreIsDebuggerPresent() && false)
			{
				trace("CfxRecvFrom (from %i %s) %i bytes on %p\n", netID, addr, length, (void*)s);
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

int __stdcall CfxSendTo(SOCKET s, char* buf, int len, int flags, sockaddr* to, int tolen)
{
	sockaddr_in* toIn = (sockaddr_in*)to;

	if (s == g_gameSocket)
	{
		if (toIn->sin_addr.S_un.S_un_b.s_b1 == 0xC0 && toIn->sin_addr.S_un.S_un_b.s_b2 == 0xA8)
		{
			g_pendSendVar = 0;

			if (CoreIsDebuggerPresent() && false)
			{
				trace("CfxSendTo (to internal address %i) %i b (from thread 0x%x)\n", (htonl(toIn->sin_addr.s_addr) & 0xFFFF) ^ 0xFEED, len, GetCurrentThreadId());
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

			trace("CfxSendTo (to %s) %i b\n", publicAddr, len);
		}

		g_netLibrary->RoutePacket(buf, len, (uint16_t)((htonl(toIn->sin_addr.s_addr)) & 0xFFFF) ^ 0xFEED);

		return len;
	}

	return sendto(s, buf, len, flags, to, tolen);
}

int __stdcall CfxBind(SOCKET s, sockaddr* addr, int addrlen)
{
	sockaddr_in* addrIn = (sockaddr_in*)addr;

	trace("binder on %i is %p\n", htons(addrIn->sin_port), s);

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

int __stdcall CfxGetSockName(SOCKET s, struct sockaddr* name, int* namelen)
{
	int retval = getsockname(s, name, namelen);

	sockaddr_in* addrIn = (sockaddr_in*)name;

	if (s == g_gameSocket && wcsstr(GetCommandLine(), L"cl2"))
	{
		addrIn->sin_port = htons(6672);
	}

	return retval;
}

int __stdcall CfxSelect(_In_ int nfds, _Inout_opt_ fd_set FAR* readfds, _Inout_opt_ fd_set FAR* writefds, _Inout_opt_ fd_set FAR* exceptfds, _In_opt_ const struct timeval FAR* timeout)
{
	bool shouldAddSocket = false;

	if (readfds)
	{
		for (int i = 0; i < readfds->fd_count; i++)
		{
			if (readfds->fd_array[i] == g_gameSocket)
			{
				memmove(&readfds->fd_array[i + 1], &readfds->fd_array[i], readfds->fd_count - i - 1);
				readfds->fd_count -= 1;
				nfds--;

				if (g_netLibrary->WaitForRoutedPacket((timeout) ? ((timeout->tv_sec * 1000) + (timeout->tv_usec / 1000)) : INFINITE))
				{
					shouldAddSocket = true;
				}
			}
		}
	}

	//FD_ZERO(readfds);

	if (nfds > 0)
	{
		nfds = select(nfds, readfds, writefds, exceptfds, timeout);
	}

	if (shouldAddSocket)
	{
		FD_SET(g_gameSocket, readfds);

		nfds += 1;
	}

	return nfds;
}

#include <CoreConsole.h>

#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

static hook::cdecl_stub<bool()> isNetworkHost([]()
{
	return hook::get_pattern("33 DB 38 1D ? ? ? ? 75 1B 38 1D", -6);
});

static HookFunction initFunction([]()
{
	g_netLibrary = NetLibrary::Create();

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

		static uint32_t lastHostSend = timeGetTime();

		if ((timeGetTime() - lastHostSend) > 1500)
		{
			lastHostState = false;
			lastHostSend = timeGetTime();
		}
	});

	OnGameFrame.Connect([]()
	{
		g_netLibrary->RunFrame();
	});

	//g_netLibrary->SetBase(GetTickCount());

	// set base to the ROS ID as that's the default gamer handle value
	// this needs patching, otherwise rlJoinSessionTask::Configure will complain that the alleged session host
	// is not in the list of gamers in the session
	auto hModule = GetModuleHandleW(L"ros-patches-rdr3.dll");
	
	if (hModule)
	{
		auto hProc = (uint64_t(*)())GetProcAddress(hModule, "GetAccountID");
		g_netLibrary->SetBase(static_cast<uint32_t>(hProc()));
	}
});

static hook::cdecl_stub<void(int, void*, void*)> joinOrHost([]()
{
	return hook::get_pattern("48 8D 55 30 49 83 60 10 00 44 8B F1", -32);
});

// 
static bool GetLocalPeerAddress(int localPlayerIdx, netPeerAddress* out)
{
	memset(out, 0, sizeof(*out));

	out->peerId.val = g_netLibrary->GetServerBase() | ((uint64_t)g_netLibrary->GetServerBase() << 32);
	*(uint64_t*)&out->gamerHandle.handle[0] = g_netLibrary->GetServerBase();
	*(uint8_t*)&out->gamerHandle.handle[10] = 3;
	out->localAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	out->localAddr.port = 6672;
	out->relayAddr.ip.addr = 0xFFFFFFFF;
	out->relayAddr.port = 0;
	out->publicAddr.ip.addr = 0x7F000001;
	out->publicAddr.port = 0;

	for (int i = 0; i < sizeof(out->peerKey) / sizeof(uint32_t); i++)
	{
		*((uint32_t*)out->peerKey + i) = g_netLibrary->GetServerBase();
	}

	out->hasPeerKey = true;

	static_assert(offsetof(netPeerAddress, hasPeerKey) == 56, "invalid");

	for (int i = 0; i < 4; i++)
	{
		out->unk.unks[i].addr.ip.addr = 0xFFFFFFFF;
	}

	return true;
}

static bool GetGamerHandle(int localPlayerIdx, rlGamerHandle* out)
{
	memset(out->handle, 0, sizeof(out->handle));
	*(uint64_t*)&out->handle[0] = g_netLibrary->GetServerBase();
	*(uint8_t*)&out->handle[10] = 3;

	return true;
}

static bool GetLocalPeerId(netPeerId* id)
{
	id->val = g_netLibrary->GetServerBase() | ((uint64_t)g_netLibrary->GetServerBase() << 32);

	return true;
}

static bool GetPublicIpAddress(netIpAddress* out)
{
	out->addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;

	return true;
}

struct RelayAddress
{
	netSocketAddress addr1;
	netSocketAddress addr2;
	int unk;
	int unk2;
	uint8_t type;
};

static RelayAddress gRelayAddress;

static RelayAddress* GetMyRelayAddress()
{
	gRelayAddress.addr1.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	gRelayAddress.addr1.port = 6672;

	gRelayAddress.addr2.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	gRelayAddress.addr2.port = 6672;

	gRelayAddress.type = 0;

	return &gRelayAddress;
}

struct P2PCryptKey
{
	uint8_t key[32];
	bool inited;
};

static bool InitP2PCryptKey(P2PCryptKey* struc)
{
	for (int i = 0; i < sizeof(struc->key) / sizeof(uint32_t); i++)
	{
		*((uint32_t*)struc->key + i) = g_netLibrary->GetServerBase();
	}

	struc->inited = true;

	return true;
}

static bool ZeroUUID(uint64_t* uuid)
{
	*uuid = 2;
	return true;
}

static bool(*origSendGamer)(char* a1, uint32_t pidx, uint32_t a3, void* null, void* msg, void* a6, void* a7);

static void* curGamer;
static uint32_t playerCountOffset;
static uint32_t playerListOffset;
static uint32_t backwardsOffset;

static bool SendGamerToMultiple(char* a1, uint32_t pidx, uint32_t a3, void* null, void* msg, void* a6, void* a7)
{
	char* session = (a1 - backwardsOffset);
	int count = *(int*)(session + playerCountOffset);
	int** list = (int**)(session + playerListOffset);

	for (int i = 0; i < count; i++)
	{
		auto p = list[i];

		/*if (p == curGamer)
		{
			continue;
		}*/

		origSendGamer(a1, *p, a3, null, msg, a6, a7);
	}

	return true;
}

static uint8_t* seamlessOff;

static void SetSeamlessOn(bool)
{
	*seamlessOff = 1;
}

static bool ReadSession(void* self, void* parTree, rlSessionInfo* session)
{
	if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
	{
		return false;
	}

	session->sessionToken.token = 2;

	auto out = &session->peerAddress;
	memset(out, 0, sizeof(*out));

	out->peerId.val = g_netLibrary->GetHostBase() | ((uint64_t)g_netLibrary->GetHostBase() << 32);
	*(uint64_t*)&out->gamerHandle.handle[0] = g_netLibrary->GetHostBase();
	*(uint8_t*)&out->gamerHandle.handle[10] = 3;
	out->localAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
	out->localAddr.port = 6672;
	out->relayAddr.ip.addr = 0xFFFFFFFF;
	out->relayAddr.port = 0;
	out->publicAddr.ip.addr = 0x7F000001;
	out->publicAddr.port = 0;

	for (int i = 0; i < sizeof(out->peerKey) / sizeof(uint32_t); i++)
	{
		*((uint32_t*)out->peerKey + i) = g_netLibrary->GetHostBase();
	}

	out->hasPeerKey = true;

	for (int i = 0; i < 4; i++)
	{
		out->unk.unks[i].addr.ip.addr = 0xFFFFFFFF;
	}

	char sessionBlob[176];
	((void(*)(rlSessionInfo*, char*, size_t, size_t*))0x1427048AC)(session, sessionBlob, 161, nullptr);

	trace("tryna join %s\n", sessionBlob);

	return true;
}

static void DumpHex(const void* data, size_t size) {
	char ascii[17];
	size_t i, j;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		trace("%02X ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		}
		else {
			ascii[i % 16] = '.';
		}
		if ((i + 1) % 8 == 0 || i + 1 == size) {
			trace(" ");
			if ((i + 1) % 16 == 0) {
				trace("|  %s \n", ascii);
			}
			else if (i + 1 == size) {
				ascii[(i + 1) % 16] = '\0';
				if ((i + 1) % 16 <= 8) {
					trace(" ");
				}
				for (j = (i + 1) % 16; j < 16; ++j) {
					trace("   ");
				}
				trace("|  %s \n", ascii);
			}
		}
	}
}

static bool ParseAddGamer(void* a1, void* a2, void* gamer)
{
	auto rv = ((bool(*)(void*, void*, void*))0x1426A77BC)(a1, a2, gamer);

	if (rv)
	{
		trace("--- ADD GAMER CMD ---\n");
		DumpHex(gamer, 384);
	}

	return rv;
}

static void LogStubLog1(void* stub, const char* type, const char* format, ...)
{
	if (type && format)
	{
		char buffer[4096];
		va_list ap;
		va_start(ap, format);
		vsnprintf(buffer, 4096, format, ap);
		va_end(ap);

		trace("%s: %s\n", type, buffer);
	}
}

#include <MinHook.h>

static int(*of1)(int*, void*);

static int f1(int* a, void* b)
{
	auto rv = of1(a, b);

	if (CoreIsDebuggerPresent() && false)
	{
		trace("RECV PKT %d\n", *a);
	}

	return rv;
}

static int(*of2)(int, void*);

static int f2(int a, void* b)
{
	auto rv = of2(a, b);

	if (CoreIsDebuggerPresent() && false)
	{
		trace("SEND PKT %d\n", a);
	}

	return rv;
}

template<typename T, T Value>
static T Return()
{
	return Value;
}

static void* rlPresence__m_GamerPresences;

static hook::cdecl_stub<void(void*)> _rlPresence_GamerPresence_Clear([]()
{
	return hook::get_call(hook::get_pattern("48 89 5D 38 48 89 5D 40 48 89 5D 48 E8", 12));
});

static hook::cdecl_stub<void(int)> _rlPresence_refreshSigninState([]()
{
	return hook::get_pattern("48 8D 54 24 20 48 69 F8 30 01 00 00 48 8D 05", -0x35);
});

static hook::cdecl_stub<void(int)> _rlPresence_refreshNetworkStatus([]()
{
	return hook::get_pattern("45 33 FF 8B DE EB 0F 48 8D", -0x7D);
});

// unused if we use sc sessions
#if 0
static void* curMigrate;
static char* g_session;

static void DoSessionMigrateToSelf()
{
	// pretend CmdMigrate succeeded
	InterlockedExchange((uint32_t*)((char*)curMigrate + 0xDC), 3); // success?
	*(uint32_t*)((char*)curMigrate + 0xE0) = 0;

	// TODO: is this only going to run on a local peer?
	rlSessionInfo* sessionInfo = (rlSessionInfo*)(g_session + 384);

	netPeerAddress peerAddress;
	GetLocalPeerAddress(0, &peerAddress);

	sessionInfo->peerAddress = peerAddress;
}

static void* (*g_orig_rlSessionEventMigrateEnd)(void* self, const rlGamerInfo& info, bool a3, int a4, int a5);

static void* rlSessionEventMigrateEnd_stub(void* self, const rlGamerInfo& info, bool a3, int a4, int a5)
{
	std::unique_ptr<NetBuffer> msgBuffer(new NetBuffer(64));

	msgBuffer->Write<uint32_t>((info.peerAddress.localAddr.ip.addr & 0xFFFF) ^ 0xFEED);
	msgBuffer->Write<uint32_t>(static_cast<uint32_t>(info.peerAddress.peerId.val));

	g_netLibrary->SendReliableCommand("msgHeHost", msgBuffer->GetBuffer(), msgBuffer->GetCurLength());

	if (isNetworkHost())
	{
		static uint64_t state;

		((void(*)(int, int, int, void*, uint64_t, void*, void*, void*))0x1426BB634)(
			0,
			*(uint32_t*)(g_session + 540),
			*(uint32_t*)(g_session + 540) - *(uint32_t*)(g_session + 15996),
			g_session + 560,
			*(uint64_t*)(g_session + 552),
			g_session + 384,
			g_session + 888,
			&state
		);

		// 1207.69 again
		((void(*)())0x14232206C)(); // reset net time sync on host
	}

	return g_orig_rlSessionEventMigrateEnd(self, info, a3, a4, a5);
}

static void* sessionCtor(char* a)
{
	g_session = a;

	return ((void* (*)(void*))0x1426B3CE8)(a);
}
#endif

static std::string g_quitMsg;

static void WINAPI ExitProcessReplacement(UINT exitCode)
{
	if (g_netLibrary)
	{
		g_netLibrary->Disconnect((g_quitMsg.empty()) ? "Exiting" : g_quitMsg.c_str());
		g_netLibrary->FinalizeDisconnect();
	}

	TerminateProcess(GetCurrentProcess(), exitCode);
}

static hook::cdecl_stub<uint32_t()> _getCurrentTransitionState([]()
{
	return hook::get_pattern("33 C0 48 85 C9 74 03 8B 41 70 C3", -7);
});

static hook::cdecl_stub<bool(uint32_t)> _transitionToState([]()
{
	return hook::get_pattern("75 04 83 60 10 00 40 B7 01 E9", -0x74);
});

static bool g_initedPlayer;

static void(*g_origHandleInitPlayerResult)(void* mgr, void* status, void* reader);

static void HandleInitPlayerResultStub(void* mgr, void* status, void* reader)
{
	g_origHandleInitPlayerResult(mgr, status, reader);

	g_initedPlayer = true;
}

#include <scrEngine.h>
#include <scrThread.h>

static struct : GtaThread
{
	virtual void DoRun() override
	{
	}
} fakeThread;

static HookFunction hookFunction([]()
{
	static ConsoleCommand quitCommand("quit", [](const std::string& message)
	{
		g_quitMsg = message;
		ExitProcess(-1);
	});

#if 0
	hook::call(0x1422E4B72, sessionCtor);

	MH_Initialize();
	MH_CreateHook((void*)0x1422357B8, f2, (void**)&of2);
	MH_CreateHook((void*)0x1422355B4, f1, (void**)&of1);
	MH_EnableHook(MH_ALL_HOOKS);

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("48 89 49 08 41 8A F0 48 83 C1 20 48 8B FA E8", -0x33), rlSessionEventMigrateEnd_stub, (void**)&g_orig_rlSessionEventMigrateEnd);
	MH_EnableHook(MH_ALL_HOOKS);
#endif

	// exitprocess -> terminateprocess
	MH_Initialize();
	MH_CreateHookApi(L"kernel32.dll", "ExitProcess", ExitProcessReplacement, nullptr);
	MH_CreateHook(hook::get_pattern("48 8B F9 48 89 44 24 20 41 FF 52 28 85", -0x22), HandleInitPlayerResultStub, (void**)&g_origHandleInitPlayerResult);
	MH_EnableHook(MH_ALL_HOOKS);

	hook::iat("ws2_32.dll", CfxSendTo, 20);
	hook::iat("ws2_32.dll", CfxRecvFrom, 17);
	hook::iat("ws2_32.dll", CfxBind, 2);
	hook::iat("ws2_32.dll", CfxSelect, 18);
	hook::iat("ws2_32.dll", CfxGetSockName, 6);

	//hook::jump(0x1406B50E8, LogStubLog1);

	{
		auto getLocalPeerAddress = hook::get_pattern<char>("48 8B D0 80 78 18 02 75 1D", -0x32);
		hook::jump(getLocalPeerAddress, GetLocalPeerAddress);
		hook::jump(hook::get_call(getLocalPeerAddress + 0x28), GetLocalPeerId);
		//hook::jump(hook::get_call(getLocalPeerAddress + 0x2D), GetMyRelayAddress);
		//hook::jump(hook::get_call(getLocalPeerAddress + 0x62), GetPublicIpAddress);
		hook::jump(hook::get_call(getLocalPeerAddress + 0xE1), GetGamerHandle);
		hook::jump(hook::get_call(getLocalPeerAddress + 0x102), InitP2PCryptKey);
		//hook::call(0x14266F66C, InitP2PCryptKey);
		//hook::call(0x14267B5A0, InitP2PCryptKey);
	}

	//
	//hook::call(0x1426E100B, ParseAddGamer);

	// all uwuids be 2
	hook::call(hook::get_pattern("B9 03 00 00 00 B8 01 00 00 00 87 83", -85), ZeroUUID);

	// get session for find result
	// 1207.58
	hook::jump(hook::get_pattern("48 85 C0 74 31 80 38 00 74 2C 45", -0x1B), ReadSession);

	seamlessOff = hook::get_address<uint8_t*>(hook::get_pattern("33 DB 38 1D ? ? ? ? 75 1B 38 1D", 4));

	// skip seamless host for is-host call
	//hook::put<uint8_t>(hook::get_pattern("75 1B 38 1D ? ? ? ? 74 36"), 0xEB);

	rlPresence__m_GamerPresences = hook::get_address<void*>(hook::get_pattern("48 8D 54 24 20 48 69 F8 30 01 00 00 48 8D 05", 0x44 - 0x35));

	static int tryHostStage = 0;

	static bool gameLoaded;

	Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([]()
	{
		gameLoaded = true;
	});

	OnKillNetwork.Connect([](const char*)
	{
		gameLoaded = false;
		g_initedPlayer = false;
	});

	OnMainGameFrame.Connect([]()
	{
		if (!gameLoaded)
		{
			return;
		}

		switch (tryHostStage)
		{
		case 0:
		{
			// update presence
			_rlPresence_GamerPresence_Clear(rlPresence__m_GamerPresences);
			_rlPresence_refreshSigninState(0);
			_rlPresence_refreshNetworkStatus(0);

			tryHostStage = 2;
			break;
		}
		case 1:
			// wait for transition
			if (_getCurrentTransitionState() == 0)
			{
				tryHostStage = 2;
			}

			break;

		case 2:
		{
			auto lastThread = rage::scrEngine::GetActiveThread();
			rage::scrEngine::SetActiveThread(&fakeThread);

			// transition to mp
			_transitionToState(0x73040199);

			rage::scrEngine::SetActiveThread(lastThread);

			tryHostStage = 3;
			break;
		}
		case 3:
			// wait for transition
			if (_getCurrentTransitionState() == 0x73040199)
			{
				tryHostStage = 4;
			}
			else if (_getCurrentTransitionState() == 0x1D94DE8C || _getCurrentTransitionState() == 0)
			{
				tryHostStage = 2;
			}

			break;

		case 4:
			if (g_initedPlayer)
			{
				tryHostStage = 5;
			}
			else if (_getCurrentTransitionState() == 0x1D94DE8C || _getCurrentTransitionState() == 0)
			{
				tryHostStage = 2;
			}

			break;

		case 5:
			static char sessionIdPtr[48];
			memset(sessionIdPtr, 0, sizeof(sessionIdPtr));
			joinOrHost(0, nullptr, sessionIdPtr);

			Instance<ICoreGameInit>::Get()->SetVariable("networkInited");
			tryHostStage = 6;

			break;
		}
	});

	static ConsoleCommand hhh("hhh", []()
	{
		tryHostStage = 0;
	});

	OnKillNetworkDone.Connect([]()
	{
		tryHostStage = 0;
	});

	// rlSession::InformPeersOfJoiner bugfix: reintroduce loop (as in, remove break; statement)
	// (by handling the _called function_ and adding the loop in a wrapper there)
	{
		auto location = hook::get_pattern<char>("4C 63 83 ? ? 00 00 4D 85 C0 7E 4F 33");

		playerCountOffset = *(uint32_t*)(location + 3);
		playerListOffset = *(uint32_t*)(location + 14 + 3);
		backwardsOffset = *(uint32_t*)(location + 62 + 3);

		hook::set_call(&origSendGamer, location + 86);

		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				mov(rax, (uintptr_t)&curGamer);
				mov(qword_ptr[rax], rsi);

				mov(rax, (uintptr_t)SendGamerToMultiple);
				jmp(rax);
			}
		} stub;

		hook::call(location + 86, stub.GetCode());
	}

#if 0
	// rage::rlMigrateSessionTask::Commence bugfix: reintroduce success state for local player
	// migrating to themselves (rlSession::CmdMigrate was removed, and this code was broken in result)
	{
		static struct : jitasm::Frontend
		{
			void InternalMain() override
			{
				mov(rax, (uintptr_t)&curMigrate);
				mov(qword_ptr[rax], rbx);

				mov(rax, (uintptr_t)DoSessionMigrateToSelf);
				jmp(rax);
			}
		} stub;

		auto location = hook::get_pattern<char>("E9 AA 00 00 00 48 8D 4C 24 60 E8 ? ? ? ? E9");
		hook::call(location + 10, stub.GetCode());
		hook::jump(location + 15, location - 10);
	}
#endif

	// don't allow tunable download requests to be considered pending
	hook::jump(hook::get_pattern("44 8B C1 44 0F B7 50 40 45 85 D2 74 18", -0x15), Return<int, 0>);

	// test: don't allow setting odd seamless mode
	//hook::jump(hook::get_call(hook::get_pattern("B1 01 E8 ? ? ? ? 80 3D", 2)), SetSeamlessOn);

	// always not seamless
	//hook::jump(hook::get_call(hook::get_pattern("84 C0 0F 84 2C 01 00 00 E8", 8)), Return<int, 1>);

	// mp cond
	hook::jump(hook::get_pattern("75 30 81 FB C7 EC 67 C1", -0xBC), Return<int, 1>); // 140E0E14C

	// reason 10
	hook::jump(hook::get_pattern("8A 81 21 20 00 00 C3"), Return<int, 1>);

	// reason 12
	hook::jump(hook::get_pattern("32 C0 45 33 C0 83 B9 D8 3F 00 00 03 74 09"), Return<int, 1>);

	// has finished loading unlocks (as these come from rtp?)
	hook::jump(hook::get_pattern("33 C0 39 41 18 74 11 F6 81 B4 00 00"), Return<int, 1>); // 1408A1014

	// skip cash/inventory
	hook::jump(hook::get_pattern("75 21 4C 8D 0D ? ? ? ? 41 B8 30 10 00 10", -0x21), Return<int, 2>); // 1423E93A8
	hook::jump(hook::get_pattern("A9 FD FF FF FF 75 64 48 8B 0D", -0x3A), Return<int, 2>); // 0x1423FAB4C

	// skip poker
	hook::jump(hook::get_pattern("48 83 EC 28 48 8B 0D ? ? ? ? E8 ? ? ? ? F6 D8 1B C0 83 C0 02"), Return<int, 2>); // 0x1423E9338
	hook::jump(hook::get_pattern("B8 02 00 00 00 EB 1F 38 91", -0x22), Return<int, 2>); // 0x1423FAAFC

	// don't stop unsafe network scripts
	hook::jump(hook::get_pattern("83 7B 10 02 74 21 48 8B CB E8", -0x35), Return<int, 0>); // 0x140E8A58C

	// pretend inventory net check (also some other subsystems but mainly inventory) is always SP
	hook::jump(hook::get_pattern("74 03 B0 01 C3 48 8B 0D", -7), Return<int, 0>);
});
