#include <StdInc.h>

#include <Hooking.h>
#include <NetLibrary.h>

static NetLibrary* g_netLibrary;

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

			if (CoreIsDebuggerPresent())
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

			if (CoreIsDebuggerPresent())
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

// rline
struct netPeerId
{
	uint64_t val;
};

struct rlGamerHandle
{
	uint8_t handle[16];
};

struct netIpAddress
{
	union
	{
		uint32_t addr;
		uint8_t bytes[4];
	};
};

struct netSocketAddress
{
	netIpAddress ip;
	uint16_t port;
};

struct netPeerUnkStructInner
{
	netSocketAddress addr;
	uint32_t unk;
};

struct netPeerUnkStruct
{
	netPeerUnkStructInner unks[4];
	int numUnks;
};

struct netPeerAddress
{
	netPeerId peerId;
	rlGamerHandle gamerHandle;
	uint8_t peerKey[32];
	uint8_t hasPeerKey;
	netSocketAddress relayAddr;
	netSocketAddress publicAddr;
	netSocketAddress localAddr;
	netPeerUnkStruct unk;
	int natType;
};

struct rlSessionToken
{
	uint64_t token;
};

struct rlSessionInfo
{
	rlSessionToken sessionToken;
	netPeerAddress peerAddress;
};

static bool g_connected;
#include <CoreConsole.h>

static void ConnectTo(const std::string& hostnameStr)
{
	if (g_connected)
	{
		trace("Ignoring ConnectTo because we're already connecting/connected.\n");
		return;
	}

	g_connected = true;

	g_netLibrary->ConnectToServer(hostnameStr);
}

#include <ICoreGameInit.h>
#include <nutsnbolts.h>

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

static hook::cdecl_stub<bool()> isNetworkHost([]()
{
	// 1207.58
	return (void*)0x14221DAF0;
});

static HookFunction initFunction([]()
{
	g_netLibrary = NetLibrary::Create();
	g_netLibrary->SetPlayerName("suka blyaaaaat!");

// 	g_netLibrary->OnInitReceived.Connect([](NetAddress)
// 	{
// 		g_netLibrary->DownloadsComplete();
// 	});

	g_netLibrary->OnStateChanged.Connect([](NetLibrary::ConnectionState curState, NetLibrary::ConnectionState lastState)
	{
		if (curState == NetLibrary::CS_ACTIVE)
		{
			ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

			if (!gameInit->GetGameLoaded())
			{
				trace("Triggering LoadGameFirstLaunch()\n");

				gameInit->LoadGameFirstLaunch([]()
				{
					// download frame code
					Sleep(1);

					return g_netLibrary->AreDownloadsComplete();
				});
			}
			else
			{
				trace("Triggering ReloadGame()\n");

				gameInit->ReloadGame();
			}
		}
	});

	g_netLibrary->OnConnectionError.Connect([](const char* e)
	{
		GlobalError("%s", e);
	});

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
				writeReliable(0xB3EA30DE, (char*)&base, sizeof(base));
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

	static ConsoleCommand connectCommand("connect", [](const std::string& server)
	{
		ConnectTo(server);
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
	*uuid = g_netLibrary->GetServerBase();
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

static void SetSeamlessOn(bool)
{
	*(uint8_t*)0x1450DB1BD = 1;
}

static bool ReadSession(void* self, void* parTree, rlSessionInfo* session)
{
	if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
	{
		return false;
	}

	session->sessionToken.token = g_netLibrary->GetHostBase();

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

	return true;
}

static HookFunction hookFunction([]()
{
	hook::iat("ws2_32.dll", CfxSendTo, 20);
	hook::iat("ws2_32.dll", CfxRecvFrom, 17);
	hook::iat("ws2_32.dll", CfxBind, 2);
	hook::iat("ws2_32.dll", CfxSelect, 18);
	hook::iat("ws2_32.dll", CfxGetSockName, 6);

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

	// all uwuids be 2
	hook::jump(hook::get_pattern("80 3D ? ? ? ? 00 75 48 48 8D 0D ? ? ? ? E8", -0x15), ZeroUUID);
	//hook::call(0x1426D3E64, ZeroUUID);

	// get session for find result
	// 1207.58
	hook::jump(0x1426F3A58, ReadSession);

	// skip seamless host for is-host call
	hook::put<uint8_t>(0x14221DAFE, 0xEB);

	static bool tryHost = true;

	OnMainGameFrame.Connect([]()
	{
		if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			return;
		}

		if (tryHost)
		{
			// update presence
			//((void(*)(int))0x1426F40F4)(0);

			static char outBuf[48];
			joinOrHost(0, nullptr, outBuf);

			tryHost = false;
		}
	});

	static ConsoleCommand hhh("hhh", []()
	{
		tryHost = true;
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

	// test: don't allow setting odd seamless mode
	hook::jump(0x1422286A8, SetSeamlessOn);
});
