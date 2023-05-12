/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "Hooking.h"
#include "NetLibrary.h"

#include <nutsnbolts.h>
#include <ICoreGameInit.h>

#include <MinHook.h>

#include <GameInit.h>

#include <scrEngine.h>
#include <ScriptEngine.h>

#include <CoreConsole.h>

#include <Error.h>

#include <CrossBuildRuntime.h>

#include <CfxState.h>
#include <HostSharedData.h>

NetLibrary* g_netLibrary;

#include <ws2tcpip.h>

static bool* g_isNetGame;

static SOCKET g_gameSocket;
static int lastReceivedFrom;

static ULONG g_pendSendVar;

int __stdcall CfxRecvFrom(SOCKET s, char * buf, int len, int flags, sockaddr * from, int * fromlen)
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

			/*if (CoreIsDebuggerPresent())
			{
				trace("CfxRecvFrom (from %i %s) %i bytes on %p\n", netID, addr, length, (void*)s);
			}*/

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

int __stdcall CfxSendTo(SOCKET s, char * buf, int len, int flags, sockaddr * to, int tolen)
{
	sockaddr_in* toIn = (sockaddr_in*)to;

	if (s == g_gameSocket)
	{
		if (toIn->sin_addr.S_un.S_un_b.s_b1 == 0xC0 && toIn->sin_addr.S_un.S_un_b.s_b2 == 0xA8)
		{
			g_pendSendVar = 0;

			/*if (CoreIsDebuggerPresent())
			{
				trace("CfxSendTo (to internal address %i) %i b (from thread 0x%x)\n", (htonl(toIn->sin_addr.s_addr) & 0xFFFF) ^ 0xFEED, len, GetCurrentThreadId());
			}*/
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

int __stdcall CfxBind(SOCKET s, sockaddr * addr, int addrlen)
{
	sockaddr_in* addrIn = (sockaddr_in*)addr;

	if (htons(addrIn->sin_port) == 6672)
	{
		if (wcsstr(GetCommandLine(), L"cl2"))
		{
			addrIn->sin_port = htons(6673);
		}

		addrIn->sin_addr.s_addr = inet_addr("127.0.0.1");
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

static int(__stdcall* g_oldSelect)(int nfds, fd_set* readfds, fd_set* writefds, fd_set* exceptfds, const timeval* timeout);

int __stdcall CfxSelect(_In_ int nfds, _Inout_opt_ fd_set FAR *readfds, _Inout_opt_ fd_set FAR *writefds, _Inout_opt_ fd_set FAR *exceptfds, _In_opt_ const struct timeval FAR *timeout)
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
		nfds = g_oldSelect(nfds, readfds, writefds, exceptfds, timeout);
	}

	if (shouldAddSocket)
	{
		FD_SET(g_gameSocket, readfds);

		nfds += 1;
	}

	return nfds;
}

bool GetOurSystemKey(char* systemKey);

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <strsafe.h>
#include <NetworkPlayerMgr.h>

template<int Build>
struct rlSessionInfo
{
	uint8_t sessionId[16];
	PeerAddress<Build> addr;
};

template<int Build>
struct rlSessionDetail
{
	PeerAddress<Build> lanAddr; // ???
	uint64_t pad;
	rlSessionInfo<Build> addr;
	uint32_t unkVal;
	char pad2[284]; // seriously this struct is weird - even in 1032
	char systemKey[16];
};

static_assert(sizeof(netPeerAddress::Impl2060) == (40 + 8 + 8 + 8), "ScInAddr size seems bad...");
static_assert(sizeof(rlSessionInfo<2060>) == (56 + 8 + 8 + 8), "rlSessionInfo size seems bad...");
static_assert(offsetof(rlSessionDetail<2372>, unkVal) == 216, "sessionDetail<2372> unkVal");
static_assert(offsetof(rlSessionDetail<1604>, systemKey) == 424, "sessionDetail<1604> 'systemKey'");
static_assert(offsetof(rlSessionDetail<2372>, systemKey) == 504, "sessionDetail<2372> 'systemKey'");

template<int Build>
bool rlSessionManager_QueryDetail(void*, void*, void* us, int* unkInt, bool something, int* a, rlSessionInfo<Build>* in, void*, rlSessionDetail<Build>* out, int* outSuccess, int* outStatus) // out might be the one before, or even same as in, dunno
{
	uint64_t sysKey = in->addr.unkKey1() ^ 0xFEAFEDE;

	memcpy(out->systemKey, in->sessionId, sizeof(out->systemKey));
	*(uint64_t*)(&out->systemKey[8]) = 0;

	if constexpr (Build >= 2245)
	{
		auto base = ((netPeerAddress::Impl2372*)&in->addr)->peerId.val;
		out->pad = base;
	}
	else
	{
		out->pad = sysKey;
	}
	out->lanAddr = in->addr;
	out->addr = *in;
	out->unkVal = 1;
	*outStatus = 3;
	*outSuccess = 1;

	return true;
}

static void(*g_origMigrateCopy)(void*, void*);

template<int Build>
void MigrateSessionCopy(char* target, char* source)
{
	g_origMigrateCopy(target, source);

	auto sessionAddress = reinterpret_cast<rlSessionInfo<Build>*>(target - 16);
	
	std::unique_ptr<net::Buffer> msgBuffer(new net::Buffer(64));

	msgBuffer->Write<uint32_t>((sessionAddress->addr.localAddr().ip.addr & 0xFFFF) ^ 0xFEED);
	msgBuffer->Write<uint32_t>(sessionAddress->addr.unkKey1());

	g_netLibrary->SendReliableCommand("msgHeHost", reinterpret_cast<const char*>(msgBuffer->GetBuffer()), msgBuffer->GetCurOffset());
}

static hook::cdecl_stub<bool()> isNetworkHost([] ()
{
	return hook::get_call(hook::pattern("48 8B CF 48 8B 92 E8 00 00 00 E8 ? ? ? ? E8").count(1).get(0).get<void>(15));
});

static bool* didPresenceStuff;

static hook::cdecl_stub<void()> doPresenceStuff([] ()
{
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		return hook::pattern("33 DB 38 1D ? ? ? ? 0F 85 ? ? ? ? E8").count(1).get(0).get<void>(-15);
	}

	return hook::pattern("32 DB 38 1D ? ? ? ? 75 24 E8").count(1).get(0).get<void>(-6);
});

static hook::cdecl_stub<void(void*, /*ScSessionAddr**/ void*, int64_t, int, void*, int)> joinGame([] ()
{
	if (xbr::IsGameBuildOrGreater<2699>())
	{
		return hook::pattern("BF 01 00 00 00 45 8B F1 45 8B F8 4C 8B E2").count(1).get(0).get<void>(-0x26);
	}

	return hook::pattern("F6 81 ? ? 00 00 01 45 8B F1 45 8B ? 4C 8B").count(1).get(0).get<void>(-0x24);
});

static hook::cdecl_stub<void(int, int, int)> hostGame([] () -> void*
{
	// below is original pattern, obfuscated since 372, so will differ per EXE now
	//return hook::get_call(hook::pattern("BA 01 00 00 00 41 B8 05 01 00 00 8B 08 E9").count(1).get(0).get<void>(13));
	//return hook::get_call(hook::pattern("48 8B 41 10 BA 01 00 00 00 41 B8 05 01 00 00").count(1).get(0).get<void>(0x11));

	// 505 has it be a xchg-type jump
	// same for 1032
	// 1737: changed
	if (!xbr::IsGameBuildOrGreater<1737>())
	{
		uint8_t* loc = hook::pattern("BA 01 00 00 00 41 B8 05 01 00 00").count(1).get(0).get<uint8_t>(11);

		if (*loc == 0xE9)
		{
			loc = hook::get_call(loc);
		}

		return loc;
	}

	if (xbr::IsGameBuild<2189>())
	{
		// 2189
		return (void*)hook::get_adjusted(0x14105DFE8);
	}

	if (xbr::IsGameBuild<2372>())
	{
		return (void*)hook::get_adjusted(0x1410646BC);
	}

	if (xbr::IsGameBuild<2545>())
	{
		return (void*)hook::get_adjusted(0x14106FF30);
	}
	
	if (xbr::IsGameBuild<2612>())
	{
		return (void*)hook::get_adjusted(0x141071468);
	}

	if (xbr::IsGameBuild<2699>())
	{
		return (void*)hook::get_adjusted(0x14107AE54);
	}

	if (xbr::IsGameBuild<2802>())
	{
		return (void*)hook::get_adjusted(0x14107A4DC);
	}

	// 1737
	//return (void*)0x141029A20;

	// 1868
	//return (void*)0x141037BCC;

	// 2060
	return (void*)hook::get_adjusted(0x1410494F8);
});

static void* getNetworkManager()
{
	static void** networkMgrPtr = nullptr;

	if (networkMgrPtr == nullptr)
	{
		networkMgrPtr = hook::get_address<void**>(hook::get_pattern("84 C0 74 2E 48 8B 0D ? ? ? ? 48 8D 54 24 20", 7));
	}

	return *networkMgrPtr;
}

struct OnlineAddress
{
	uint32_t ip1;
	uint16_t port1;
	uint32_t ip2;
	uint16_t port2;
};

static OnlineAddress* g_onlineAddress;

OnlineAddress* GetOurOnlineAddressRaw()
{
	if (!g_onlineAddress)
	{
		return nullptr;
	}

	g_onlineAddress->ip1 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	g_onlineAddress->port1 = 6672;

	g_onlineAddress->ip2 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	g_onlineAddress->port2 = 6672;

	return g_onlineAddress;
}

static hook::cdecl_stub<bool()> isSessionStarted([] ()
{
	return hook::get_call(hook::get_pattern("8B 86 ? ? 00 00 C1 E8 05 84 C2", 13));
});

// Network bail function definition changed in 2372
static void* g_NetworkBail = nullptr;
using networkBail_2372 = void((*)(void*, bool));
using networkBail_1604 = void((*)(int, int, int, int, bool));

static bool(*_isScWaitingForInit)();

#include <HostSystem.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <msgpack.hpp>

GTA_NET_EXPORT fwEvent<HostState, HostState> OnHostStateTransition;

struct HostStateHolder
{
	HostState state;

	inline bool operator==(HostState right)
	{
		return (state == right);
	}

	inline HostState operator=(HostState right)
	{
		trace("HostState transitioning from %s to %s\n", HostStateToString(state), HostStateToString(right));

		AddCrashometry("hs_state", HostStateToString(right));

		OnHostStateTransition(right, state);
		state = right;

		return right;
	}
};

bool IsWaitingForTimeSync();

struct  
{
	HostStateHolder state;
	int attempts;

	std::string hostResult;

	void handleHostResult(const std::string& str)
	{
		hostResult = str;
	}

	template<int Build>
	void process()
	{
		ICoreGameInit* cgi = Instance<ICoreGameInit>::Get();

		if (state == HS_LOADED)
		{
			if (_isScWaitingForInit())
			{
				return;
			}

			if (cgi->OneSyncEnabled)
			{
				if (IsWaitingForTimeSync())
				{
					return;
				}
			}

			// if there's no host, start hosting - if there is, start joining
			if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
			{
				state = HS_START_HOSTING;
			}
			else
			{
				state = HS_START_JOINING;
			}
		}
		else if (state == HS_START_JOINING)
		{
			static rlSessionInfo<Build> netAddr;
			memset(&netAddr, 0, sizeof(netAddr));

			if constexpr (Build < 2245)
			{
				netAddr.addr.secKeyTime() = g_netLibrary->GetHostBase() ^ 0xABCD;

				netAddr.addr.unkKey1() = g_netLibrary->GetHostBase();
				netAddr.addr.unkKey2() = g_netLibrary->GetHostBase();

				netAddr.addr.localAddr().ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.localAddr().port = 6672;

				netAddr.addr.relayAddr().ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.relayAddr().port = 6672;

				netAddr.addr.publicAddr().ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.publicAddr().port = 6672;
			}
			else
			{
				auto out = (netPeerAddress::Impl2372*)&netAddr.addr;
				out->peerId.val = (g_netLibrary->GetHostBase() | ((uint64_t)g_netLibrary->GetHostBase() << 32)) ^ 1;
				*(uint64_t*)&out->gamerHandle.handle[0] = g_netLibrary->GetHostBase();
				*(uint16_t*)&out->gamerHandle.handle[8] = 3;
				out->localAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				out->localAddr.port = 6672;
				out->relayAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				out->relayAddr.port = 6672;
				out->publicAddr.ip.addr = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				out->publicAddr.port = 6672;

				for (int i = 0; i < sizeof(out->peerKey) / sizeof(uint32_t); i++)
				{
					*((uint32_t*)out->peerKey + i) = g_netLibrary->GetHostBase();
				}

				out->hasPeerKey = true;
			}

			*(uint32_t*)&netAddr.sessionId[0] = 0x2;//g_netLibrary->GetHostBase() ^ 0xFEAFEDE;
			*(uint32_t*)&netAddr.sessionId[8] = 0xCDCDCDCD;

			joinGame(getNetworkManager(), &netAddr, 1, 1, nullptr, 0);

			state = HS_JOINING;
		}
		else if (state == HS_JOINING)
		{
			if (*g_isNetGame)
			{
				state = HS_JOINING_NET_GAME;
			}
		}
		else if (state == HS_JOINING_NET_GAME)
		{
			// two possible target states
			if (isSessionStarted())
			{
				cgi->SetVariable("networkInited");
				state = HS_JOINED;
			}
			else if (!*g_isNetGame)
			{
				state = HS_JOIN_FAILURE;
			}
		}
		else if (state == HS_JOIN_FAILURE)
		{
			if (cgi->EnhancedHostSupport)
			{
				state = HS_START_HOSTING;
			}
			else
			{
				trace("No enhanced host support is active, failing fast.\n");

				state = HS_FATAL;
			}
		}
		else if (state == HS_START_HOSTING)
		{
			if (cgi->EnhancedHostSupport)
			{
				msgpack::sbuffer nameArgs;
				msgpack::packer<msgpack::sbuffer> packer(nameArgs);

				packer.pack_array(0);

				g_netLibrary->SendNetEvent("hostingSession", std::string(nameArgs.data(), nameArgs.size()), -2);

				state = HS_WAIT_HOSTING;
			}
			else
			{
				hostGame(0, 32, 0x0);
				state = HS_HOSTING;
			}
		}
		else if (state == HS_WAIT_HOSTING || state == HS_WAIT_HOSTING_2)
		{
			if (!hostResult.empty())
			{
				std::string result = hostResult;
				hostResult = "";

				trace("received hostResult %s\n", result);

				// 'wait' is a no-op, should wait for the next response
				if (result == "wait")
				{
					state = HS_WAIT_HOSTING_2;
					return;
				}

				if (state == HS_WAIT_HOSTING_2)
				{
					if (result == "free")
					{
						// if free after a wait, restart as if loaded
						state = HS_LOADED;
						return;
					}
				}

				if (result == "conflict")
				{
					if (g_netLibrary->GetHostNetID() != 0xFFFF && g_netLibrary->GetHostNetID() != g_netLibrary->GetServerNetID())
					{
						trace("session creation conflict");

						state = HS_FATAL;
						return;
					}
				}

				// no conflict, no wait, start hosting
				hostGame(0, 32, 0x0);
				state = HS_HOSTING;
			}
		}
		else if (state == HS_HOSTING)
		{
			if (*g_isNetGame)
			{
				state = HS_HOSTING_NET_GAME;
			}
		}
		else if (state == HS_HOSTING_NET_GAME)
		{
			if (isSessionStarted())
			{
				cgi->SetVariable("networkInited");
				state = HS_HOSTED;

				if (cgi->EnhancedHostSupport)
				{
					msgpack::sbuffer nameArgs;
					msgpack::packer<msgpack::sbuffer> packer(nameArgs);

					packer.pack_array(0);

					g_netLibrary->SendNetEvent("hostedSession", std::string(nameArgs.data(), nameArgs.size()), -2);
				}
			}
			else if (!*g_isNetGame)
			{
				state = HS_FATAL;
			}
		}
		else if (state == HS_FATAL)
		{
			if (attempts < 3)
			{
				++attempts;
				state = HS_LOADED;
			}
			else
			{
				GlobalError("Could not connect to session provider. This may happen when you recently updated, but other players in the server have not. Alternately, the server accepted you, despite being full. Please try again later, or try a different server.");
				state = HS_IDLE;
			}
		}
		else if (state == HS_HOSTED || state == HS_JOINED)
		{
			int playerCount = 0;

			for (int i = 0; i < 256; i++)
			{
				// NETWORK_IS_PLAYER_ACTIVE
				if (NativeInvoke::Invoke<0xB8DFD30D6973E135, bool>(i))
				{
					++playerCount;
				}
			}

			static uint64_t mismatchStart = UINT64_MAX;

			if (isNetworkHost() && playerCount == 1 && !cgi->OneSyncEnabled && g_netLibrary->GetHostNetID() != g_netLibrary->GetServerNetID())
			{
				if (mismatchStart == UINT64_MAX)
				{
					mismatchStart = GetTickCount64();
				}
				else if ((GetTickCount64() - mismatchStart) > 7500)
				{
					state = HS_MISMATCH;
				}
			}
			else
			{
				mismatchStart = UINT64_MAX;
			}
		}
		else if (state == HS_MISMATCH)
		{
			cgi->ClearVariable("networkInited");

			if (xbr::IsGameBuildOrGreater<2372>())
			{
				int32_t args[5] = { 7, -1, -1, -1, -1 };
				((networkBail_2372)g_NetworkBail)(&args, true);
			}
			else
			{
				((networkBail_1604)g_NetworkBail)(7, -1, -1, -1, true);
			}

			state = HS_DISCONNECTING;
		}
		else if (state == HS_FORCE_DISCONNECT)
		{
			if (xbr::IsGameBuildOrGreater<2372>())
			{
				int32_t args[5] = { 7, -1, -1, -1, -1 };
				((networkBail_2372)g_NetworkBail)(&args, true);
			}
			else
			{
				((networkBail_1604)g_NetworkBail)(7, -1, -1, -1, true);
			}

			state = HS_DISCONNECTING_FINAL;
		}
		else if (state == HS_DISCONNECTING)
		{
			if (!isSessionStarted())
			{
				state = HS_LOADED;
			}
		}
		else if (state == HS_DISCONNECTING_FINAL)
		{
			if (!isSessionStarted())
			{
				cgi->OneSyncEnabled = false;

				state = HS_IDLE;
			}
		}
	}
} hostSystem;

static InitFunction hsInitFunction([]()
{
	fx::ResourceManager::OnInitializeInstance.Connect([] (fx::ResourceManager* manager)
	{
		fwRefContainer<fx::ResourceEventManagerComponent> eventComponent = manager->GetComponent<fx::ResourceEventManagerComponent>();

		if (eventComponent.GetRef())
		{
			eventComponent->OnQueueEvent.Connect([](const std::string& eventName, const std::string& eventPayload, const std::string& eventSource)
			{
				// if this is the event 'we' handle...
				if (eventName == "sessionHostResult")
				{
					// deserialize the arguments
					msgpack::unpacked msg;
					msgpack::unpack(msg, eventPayload.c_str(), eventPayload.size());

					msgpack::object obj = msg.get();

					// convert to an array
					std::vector<msgpack::object> arguments;
					obj.convert(arguments);

					hostSystem.handleHostResult(arguments[0].as<std::string>());
				}
			});
		}
	});
});

static uint16_t* g_dlcMountCount;

void Policy_BindNetLibrary(NetLibrary*);
void MumbleVoice_BindNetLibrary(NetLibrary*);
void ObjectIds_BindNetLibrary(NetLibrary*);

#include <CloneManager.h>

static hook::cdecl_stub<void(void* /* rlGamerInfo */)> _setGameGamerInfo([]()
{
	return hook::get_pattern("3A D8 0F 95 C3 40 0A DE 40", (xbr::IsGameBuildOrGreater<2699>()) ? -0x56 : -0x53);
});

template<int Build>
static rlGamerInfo<Build>** g_gamerInfo;

template<int Build>
static void RunGameFrame()
{
	GetOurOnlineAddressRaw();

	auto gi = *g_gamerInfo<Build>;

	if (!gi)
	{
		return;
	}

	static auto origNonce = gi->gamerId;
	uint64_t tgtNonce;

	if (Instance<ICoreGameInit>::Get()->OneSyncEnabled)
	{
		tgtNonce = g_netLibrary->GetServerNetID();
	}
	else
	{
		tgtNonce = origNonce;
	}

	if (gi->gamerId != tgtNonce)
	{
		gi->gamerId = tgtNonce;

		_setGameGamerInfo(gi);
	}
}

static HookFunction initFunction([]()
{
	g_netLibrary = NetLibrary::Create();

	Instance<NetLibrary>::Set(g_netLibrary);

	TheClones->BindNetLibrary(g_netLibrary);

	MumbleVoice_BindNetLibrary(g_netLibrary);

	Policy_BindNetLibrary(g_netLibrary);

	ObjectIds_BindNetLibrary(g_netLibrary);

	static NetLibrary* netLibrary;

	static bool doTickNextFrame;

	g_netLibrary->OnConnectOKReceived.Connect([] (NetAddress addr)
	{
		doTickNextFrame = true;
	});

	// host state sending
	g_netLibrary->OnBuildMessage.Connect([] (const std::function<void(uint32_t, const char*, int)>& writeReliable)
	{
		ICoreGameInit* cgi = Instance<ICoreGameInit>::Get();

		if (cgi->OneSyncEnabled)
		{
			return;
		}

		static bool lastHostState;

		// send whether or not we consider ourselves to be host
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

		// ensure we send this frequently enough
		static uint32_t lastHostSend = timeGetTime();

		if ((timeGetTime() - lastHostSend) > 1500)
		{
			lastHostState = false;
			lastHostSend = timeGetTime();
		}
	});

	g_netLibrary->AddReliableHandler("msgFrame", [](const char* data, size_t len)
	{
		net::Buffer buffer(reinterpret_cast<const uint8_t*>(data), len);
		auto idx = buffer.Read<uint32_t>();

		auto icgi = Instance<ICoreGameInit>::Get();

		uint8_t strictLockdown = 0;
		uint8_t syncStyle = 0;

		if (icgi->NetProtoVersion >= 0x202002271209)
		{
			strictLockdown = buffer.Read<uint8_t>();
		}

		if (icgi->NetProtoVersion >= 0x202011231556)
		{
			syncStyle = buffer.Read<uint8_t>();
		}

		static uint8_t lastStrictLockdown;

		if (strictLockdown != lastStrictLockdown)
		{
			if (!strictLockdown)
			{
				icgi->ClearVariable("strict_entity_lockdown");
			}
			else
			{
				icgi->SetVariable("strict_entity_lockdown");
			}

			lastStrictLockdown = strictLockdown;
		}

		icgi->SyncIsARQ = syncStyle == 1;
	}, true);

	g_netLibrary->SetBase(GetTickCount());

	if (xbr::IsGameBuildOrGreater<2699>())
	{
		static uintptr_t gamerInfoPtr = hook::get_address<uintptr_t>((char*)hook::get_call(hook::get_pattern("48 8B E8 48 85 C0 74 11 E8", 8)) + 3);
		g_gamerInfo<2372> = (decltype(g_gamerInfo<2372>))(&gamerInfoPtr);
	}
	else
	{
		auto location = hook::get_pattern("FF C8 0F 85 AC 00 00 00 48 39 35", 11);

		if (xbr::IsGameBuildOrGreater<2372>())
		{
			g_gamerInfo<2372> = hook::get_address<decltype(g_gamerInfo<2372>)>(location);
		}
		if (xbr::IsGameBuildOrGreater<2060>())
		{
			g_gamerInfo<2060> = hook::get_address<decltype(g_gamerInfo<2060>)>(location);
		}
		else
		{
			g_gamerInfo<1604> = hook::get_address<decltype(g_gamerInfo<1604>)>(location);
		}
	}

	static bool doTickThisFrame = false;

	OnGameFrame.Connect([]()
	{
		if (xbr::IsGameBuildOrGreater<2372>())
		{
			RunGameFrame<2372>();
		}
		else if (xbr::IsGameBuildOrGreater<2060>())
		{
			RunGameFrame<2060>();
		}
		else
		{
			RunGameFrame<1604>();
		}
	});

	OnCriticalGameFrame.Connect([]()
	{
		g_netLibrary->RunFrame();
	});

	OnMainGameFrame.Connect([] ()
	{
		g_netLibrary->RunMainFrame();

		static bool gameLoaded = false;
		static bool eventConnected = false;

		if (!eventConnected)
		{
			Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([] ()
			{
				gameLoaded = true;
			});

			OnKillNetwork.Connect([](const char*)
			{
				gameLoaded = false;
				Instance<ICoreGameInit>::Get()->ClearVariable("networkTimedOut");
			});

			if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
			{
				gameLoaded = true;
			}

			eventConnected = true;
		}

		if (gameLoaded && doTickThisFrame)
		{
			gameLoaded = false;

			if (*g_dlcMountCount != 132)
			{
				if (!xbr::IsGameBuildOrGreater<2060>())
				{
					// #TODO1737
					GlobalError("DLC count mismatch - %d DLC mounts exist locally, but %d are expected. Please check that you have installed all core game updates and try again.", *g_dlcMountCount, 132);

					return;
				}
			}

			hostSystem.state = HS_LOADED;
			doTickThisFrame = false;
		}

		if (Instance<ICoreGameInit>::Get()->GetGameLoaded())
		{
			if (xbr::IsGameBuildOrGreater<2372>())
			{
				hostSystem.process<2372>();			
			}
			else if (xbr::IsGameBuildOrGreater<2060>())
			{
				hostSystem.process<2060>();			
			}
			else
			{
				hostSystem.process<1604>();
			}
		}

		if (gameLoaded && doTickNextFrame)
		{
			doTickThisFrame = true;

			// redo presence stuff?
			*didPresenceStuff = false;

			// moved here as connection might succeed prior to the rline dispatcher being initialized
			doPresenceStuff();

			doTickNextFrame = false;
		}
	});
});

static uint64_t* g_globalNetSecurityKey;

bool GetOurOnlineAddress(netPeerAddress* address)
{
	memset(address, 0, sizeof(*address));
	address->secKeyTime() = g_netLibrary->GetServerBase() ^ 0xABCD;
	address->unkKey1() = g_netLibrary->GetServerBase();
	address->unkKey2() = g_netLibrary->GetServerBase();
	address->localAddr().ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->localAddr().port = 6672;
	address->relayAddr().ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->relayAddr().port = 6672;
	address->publicAddr().ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->publicAddr().port = 6672;
	//address->pad5 = 0x19;

	g_globalNetSecurityKey[0] = g_netLibrary->GetServerBase();
	g_globalNetSecurityKey[1] = g_netLibrary->GetServerBase();

	return true;
}

// 2245+
bool GetOurOnlineAddressNew(void* unk, netPeerAddress::Impl2372* address)
{
	memset(address, 0, sizeof(*address));
	address->peerId.val = g_netLibrary->GetServerBase() | ((uint64_t)g_netLibrary->GetServerBase() << 32);
	*(uint64_t*)&address->gamerHandle.handle[0] = g_netLibrary->GetServerBase();
	*(uint16_t*)&address->gamerHandle.handle[8] = 3;
	address->localAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->localAddr.port = 6672;
	address->relayAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->relayAddr.port = 6672;
	address->publicAddr.ip.addr = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->publicAddr.port = 6672;

	for (int i = 0; i < sizeof(address->peerKey) / sizeof(uint32_t); i++)
	{
		*((uint32_t*)address->peerKey + i) = g_netLibrary->GetServerBase();
	}

	address->hasPeerKey = true;

	return true;
}

static netPeerId* g_peerId;

static bool GetLocalPeerId(netPeerId* id)
{
	id->val = g_netLibrary->GetServerBase() | ((uint64_t)g_netLibrary->GetServerBase() << 32);
	g_peerId->val = id->val;

	return true;
}

static bool GetLocalPeerId2(netPeerId* id)
{
	id->val = (g_netLibrary->GetServerBase() | ((uint64_t)g_netLibrary->GetServerBase() << 32)) ^ 1;

	return true;
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

bool GetOurSystemKey(char* systemKey)
{
	// then set this stuff
	memset(systemKey, 0, 8);
	*(uint32_t*)&systemKey[0] = g_netLibrary->GetServerBase() ^ 0xFEAFEDE;

	return true;
}

bool GetOurSessionKey(char* sessionKey)
{
	*(uint64_t*)sessionKey = 2;

	return true;
}

static int ReturnFalse()
{
	return false;
}

static int ReturnTrue()
{
	return true;
}

static bool ReturnTrueAndKillThatTask(char* playerObj)
{
	char* ped = *(char**)(playerObj + 80);
	char* task = *(char**)(ped + 32); // why is this in fwEntity anyway?

	task[308] = 0;

	return true;
}

static int* g_netNewVal;

static void GetNetNewVal()
{
	*g_netNewVal = 0;
}

static void HashSecKeyAddress(uint64_t* outValue, uint32_t seed)
{
	outValue[0] = seed ^ 0xABCD;
	outValue[1] = seed ^ 0xABCD;
}

static bool(*g_makeOurSystemKey)(char* key);
static void* g_sessionKeyReturn;

bool GetOurSessionKeyWrap(char* sessionKey)
{
	// #TODO: does this make sense for 2189/2372?
	if (_ReturnAddress() == g_sessionKeyReturn)
	{
		*(uint64_t*)sessionKey = 2;

		return true;
	}

	return GetOurSystemKey(sessionKey);
}

static hook::cdecl_stub<void(void*, void*)> setUnkTimeValue([]()
{
	return hook::get_pattern("48 8B F2 48 8B D9 74 1A 48 8B", -0x14);
});

struct ncm_struct
{
	char pad[96];

	inline void SetUnkTimeValue(void* a1)
	{
		return setUnkTimeValue(this, a1);
	}
};

template<int Build>
struct netConnectionManager
{
private:
	virtual ~netConnectionManager() = 0;

public:
	SOCKET socket; // +8
	netConnectionManager<Build>* secondarySocket; // +16 // or similar..
	char m_pad[(Build >= 2372) ? 680 : 424]; // +24
	ncm_struct unkStructs[16];
};

static_assert(offsetof(netConnectionManager<1604>, unkStructs) == 448, "netConnectionManager 1604");
static_assert(offsetof(netConnectionManager<2372>, unkStructs) == 704, "netConnectionManager 2372");

template<int Build>
struct netConnectionManagerInternal
{
	netConnectionManager<Build>* socketData;
	char pad[(Build >= 2372) ? 36 : 60];
	int unk_marker;

	SOCKET GetSocket()
	{
		if (socketData->secondarySocket)
		{
			return socketData->secondarySocket->socket;
		}
		else
		{
			return socketData->socket;
		}
	}
};

template<int Build>
static netConnectionManagerInternal<Build>* g_internalNet;

template<int Build>
static netConnectionManager<Build>* g_netConnectionManager;

static bool(*g_handleQueuedSend)(void*);

static bool(*g_origCreateSendThreads)(void*, void*, int);

template<int Build>
bool CustomCreateSendThreads(netConnectionManagerInternal<Build>* a1, netConnectionManager<Build>* a2, int a3)
{
	g_internalNet<Build> = a1;
	g_netConnectionManager<Build> = a2;

	return g_origCreateSendThreads(a1, a2, a3);
}

static uint32_t(*g_receivePacket)(void*, void*);
static void(*g_handlePacket)(void*, void*, uint32_t);

void RunNetworkStuff()
{
	// handle queued sends
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		g_handleQueuedSend(g_netConnectionManager<2372>);
	}
	else
	{
		g_handleQueuedSend(g_netConnectionManager<1604>);
	}

	// set timer stuff in connection manager
	// this seems to be required to actually retain sync
	// NOTE: 505-specific (struct offsets, ..)!!
	// updated for 1103
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		for (auto& entry : g_netConnectionManager<2372>->unkStructs)
		{
			entry.SetUnkTimeValue(&g_internalNet<2372>->unk_marker);
		}
	}
	else
	{
		for (auto& entry : g_netConnectionManager<1604>->unkStructs)
		{
			entry.SetUnkTimeValue(&g_internalNet<1604>->unk_marker);
		}
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

static void(*_origLoadMeta)(const char*, bool, uint32_t);

static void WaitForScAndLoadMeta(const char* fn, bool a2, uint32_t a3)
{
	WaitForRlInit();

	return _origLoadMeta(fn, a2, a3);
}

static bool (*g_origBeforeReplayLoad)();

static bool BeforeReplayLoadHook()
{
	if (hostSystem.state == HS_HOSTED || hostSystem.state == HS_JOINED)
	{
		hostSystem.state = HS_FORCE_DISCONNECT;
	}

	if (!(hostSystem.state == HS_IDLE))
	{
		return false;
	}

	g_netLibrary->Disconnect("Entering Rockstar Editor");

	// stop scripts from this point
	Instance<ICoreGameInit>::Get()->SetVariable("networkTimedOut");

	return g_origBeforeReplayLoad();
}

static void ExitCleanly()
{
	g_quitMsg = "Exiting";
	ExitProcess(0);

	__debugbreak();
}

#include <shellapi.h>

static BOOL ShellExecuteExAHook(SHELLEXECUTEINFOA *pExecInfo)
{
	static HostSharedData<CfxState> hostData("CfxInitState");
	auto cli = const_cast<wchar_t*>(va(L"\"%s\" %s -switchcl", hostData->gameExePath, ToWide(
		pExecInfo->lpParameters ?
			pExecInfo->lpParameters : ""
	)));

	STARTUPINFOW si = { 0 };
	si.cb = sizeof(si);

	PROCESS_INFORMATION pi;
	CreateProcessW(NULL, cli, NULL, NULL, FALSE, CREATE_BREAKAWAY_FROM_JOB, NULL, NULL, &si, &pi);
	
	return TRUE;
}

static int* g_clipsetManager_networkState;
static void (*g_orig_fwClipSetManager_StartNetworkSession)();

static void fwClipSetManager_StartNetworkSessionHook()
{
	if (*g_clipsetManager_networkState != 2)
	{
		g_orig_fwClipSetManager_StartNetworkSession();
	}
}

static void (*g_origPoliceScanner_Stop)(void*, int);

static void PoliceScanner_StopWrap(void* self, int a2)
{
	if (!isSessionStarted())
	{
		g_origPoliceScanner_Stop(self, a2);
	}
}

static HookFunction hookFunction([] ()
{
	MH_Initialize();

	static ConsoleCommand quitCommand("quit", [](const std::string& message)
	{
		g_quitMsg = message;
		ExitProcess(-1);
	});

	// exit game on game exit from alt-f4
	hook::call(hook::get_pattern("48 83 F8 04 75 ? 40 88", 6), ExitCleanly);

	// no netgame jumpouts in alt-f4
	hook::put<uint8_t>(hook::get_pattern("40 38 35 ? ? ? ? 74 0A 48 8B CF", 7), 0xEB);

	// fix 'restart' handling to not ask MTL to restart, but relaunch 'ourselves' (eg on settings change)
	hook::put<uint8_t>(hook::get_pattern("48 85 C9 74 15 40 38 31 74", 3), 0xEB);

	// shellexecuteexa -switch add so it can wait
	hook::iat("shell32.dll", ShellExecuteExAHook, "ShellExecuteExA");

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		char* getNewNewVal = hook::pattern("41 83 CF FF 33 DB 4C 8D 25").count(1).get(0).get<char>(45);
		g_netNewVal = (int*)(*(int32_t*)getNewNewVal + getNewNewVal + 4);

		getNewNewVal -= 0x4E;
		hook::jump(getNewNewVal, GetNetNewVal);
	}
	else
	{
		//char* getNewNewVal = hook::pattern("33 D2 41 B8 00 04 00 00 89 1D ? ? ? ? E8").count(1).get(0).get<char>(10);
		char* getNewNewVal = hook::pattern("33 D2 41 B8 00 04 00 00 89 1D").count(1).get(0).get<char>(10); // 463/505
		g_netNewVal = (int*)(*(int32_t*)getNewNewVal + getNewNewVal + 4);

		getNewNewVal -= 0x3F;
		hook::jump(getNewNewVal, GetNetNewVal);
	}

	// 2060 matches this pattern more than once, and *second* match there
	// use old pattern for <2372
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		didPresenceStuff = hook::get_address<bool*>(hook::get_pattern<char>("75 ? 40 38 3D ? ? ? ? 74 ? 48 8D 0D", 5));
	}
	else
	{
		didPresenceStuff = hook::get_address<bool*>(hook::get_pattern<char>("32 DB 38 1D ? ? ? ? 75 24 E8", 4));
	}

	hook::iat("ws2_32.dll", CfxSendTo, 20);
	hook::iat("ws2_32.dll", CfxRecvFrom, 17);
	hook::iat("ws2_32.dll", CfxBind, 2);
	g_oldSelect = hook::iat("ws2_32.dll", CfxSelect, 18);
	hook::iat("ws2_32.dll", CfxGetSockName, 6);

	// session migration, some 'inline' memcpy of the new address
	void* migrateCmd;

	if (xbr::IsGameBuildOrGreater<2372>())
	{
		migrateCmd = hook::get_pattern("48 89 81 B0 00 00 00 48 8B 87 A0 00 00 00", 0x19);
	}
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		migrateCmd = hook::get_pattern("48 8B 87 80 00 00 00 48 81 C1 A0 00 00 00", 0x12);
	}
	else
	{
		migrateCmd = hook::get_pattern("48 8B 47 78 48 81 C1 98 00 00 00 48 89 41 F8", 15);
	}


	hook::set_call(&g_origMigrateCopy, migrateCmd);
	hook::call(migrateCmd, (xbr::IsGameBuildOrGreater<2372>()) ? (void*)&MigrateSessionCopy<2372> : (xbr::IsGameBuildOrGreater<2060>()) ? (void*)&MigrateSessionCopy<2060> : &MigrateSessionCopy<1604>);

	// session key getting system key; replace with something static for migration purposes
	char* sessionKeyAddress = hook::pattern("74 15 48 8D 4C 24 78 E8").count(1).get(0).get<char>(7);
	char* fillOurSystemKey = hook::get_call(sessionKeyAddress);

	hook::set_call(&g_makeOurSystemKey, fillOurSystemKey + 14);
	g_sessionKeyReturn = sessionKeyAddress + 5;

	// we also need some pointers from this function
	char* onlineAddressFunc;

	// 2372-n (technically, 2245)
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		char* netAddressFunc = hook::get_pattern<char>("80 78 ? 02 48 8B D0 75", -0x32);
		hook::jump(netAddressFunc, GetOurOnlineAddressNew);
		g_peerId = hook::get_address<netPeerId*>(hook::get_call(netAddressFunc + 0x28) + 0x1C);
		hook::jump(hook::get_call(netAddressFunc + 0x28), GetLocalPeerId);
		hook::jump(hook::get_call(hook::get_call(netAddressFunc + 0x102) + 0x14), InitP2PCryptKey);

		onlineAddressFunc = hook::get_call(netAddressFunc + 0x2D);

		char* rlCreateUUID = hook::get_pattern<char>("48 8D 0D ? ? ? ? 48 8D 14 03", -0x31);
		bool* didNetAddressBool = hook::get_address<bool*>(rlCreateUUID + 0x48, 2, 7);
		*didNetAddressBool = true;

		hook::call(sessionKeyAddress, GetOurSessionKeyWrap);
	}
	// 2060-2215
	else if (xbr::IsGameBuildOrGreater<2060>())
	{
		char* netAddressFunc = hook::get_pattern<char>("89 79 10 48 89 39 48 89 79 08 89 69 14 66 89 79", -0x22);
		hook::jump(netAddressFunc, GetOurOnlineAddress);

		hook::jump(hook::get_call(netAddressFunc + 0x6F), HashSecKeyAddress);
		onlineAddressFunc = hook::get_call(netAddressFunc + 0x74);

		bool* didNetAddressBool = hook::get_address<bool*>(netAddressFunc + 0x1E);
		*didNetAddressBool = true;

		g_globalNetSecurityKey = hook::get_address<uint64_t*>(netAddressFunc + 0x63);

		hook::jump(fillOurSystemKey, GetOurSessionKeyWrap);
	}
	// 1604
	else
	{
		char* netAddressFunc = hook::get_pattern<char>("89 79 10 48 89 39 48 89 79 08 89 69 14 66 89 79", -0x23);
		hook::jump(netAddressFunc, GetOurOnlineAddress);

		hook::jump(hook::get_call(netAddressFunc + 0x66), HashSecKeyAddress);
		onlineAddressFunc = hook::get_call(netAddressFunc + 0x6B);

		bool* didNetAddressBool = hook::get_address<bool*>(netAddressFunc + 0x1F);
		*didNetAddressBool = true;

		g_globalNetSecurityKey = hook::get_address<uint64_t*>(netAddressFunc + 0x5A);

		hook::jump(fillOurSystemKey, GetOurSessionKeyWrap);
	}

	hook::jump(onlineAddressFunc, GetOurOnlineAddressRaw);

	onlineAddressFunc += 0x23;

	g_onlineAddress = (OnlineAddress*)(onlineAddressFunc + *(int32_t*)onlineAddressFunc + 4);

	{
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			g_NetworkBail = hook::get_pattern("48 83 EC 70 80 3D ? ? ? ? 00 C6 05", -5);
		}
		else if (xbr::IsGameBuildOrGreater<2372>())
		{
			g_NetworkBail = hook::get_pattern("80 3D ? ? ? ? 00 8B 01 48 8B D9 89", -0xD);
		}
		else
		{
			g_NetworkBail = hook::get_pattern("41 8B F1 41 8B E8 8B FA 8B D9 74 26", -0x1B);
		}
	}

	// system key *local peer id*
	if (!xbr::IsGameBuildOrGreater<2372>())
	{
		hook::jump(hook::pattern("48 83 F8 FF 75 17 48 8D 0D").count(1).get(0).get<void>(-32), GetOurSystemKey);
	}
	else
	{
		hook::jump(hook::pattern("48 83 F8 FF 75 17 48 8D 0D").count(1).get(0).get<void>(-32), GetLocalPeerId2);
	}

	// unknown obfuscated check
	hook::jump(hook::get_call(hook::pattern("84 C0 74 0F BA 03 00 00 00 48 8B CB E8 ? ? ? ? EB 0E").count(1).get(0).get<void>(12)), ReturnTrue);

	// locate address thingy
	hook::jump(hook::get_call(hook::get_pattern<char>("E8 ? ? ? ? 84 C0 0F 84 ? ? ? ? 49 8B 8E A0 00 00 00")), 
		(xbr::IsGameBuildOrGreater<2372>()) ? (void*)rlSessionManager_QueryDetail<2372> : (xbr::IsGameBuildOrGreater<2060>()) ? (void*)rlSessionManager_QueryDetail<2060> : rlSessionManager_QueryDetail<1604>);

	// temp dbg: always clone a player (to see why this CTaskMove flag is being a twat)
	hook::jump(hook::pattern("48 85 C0 74 13 ? ? ? ? 74 0D 48 85 DB 74 19").count(1).get(0).get<void>(-0x37), ReturnTrueAndKillThatTask);
	void* updateScAdvertisement = hook::pattern("48 89 44 24 20 E8 ? ? ? ? F6 D8 1B C9 83 C1").count(1).get(0).get<void>();

	// temporary(!) patch to force CGameScriptObjInfo to act as if having an unknown identifier set (as regular creation doesn't set it?! - doesn't write to it at all)
	hook::nop(hook::pattern("83 79 10 00 74 05 48 8D 41 08 C3 33 C0 C3").count(1).get(0).get<void>(4), 2);

	// semi-related: adding to a script handler checking for the above value being 0
	hook::nop(hook::pattern("FF 50 28 45 33 E4 48 85 C0 0F 85").count(1).get(0).get<void>(9), 6);

	if (xbr::IsGameBuildOrGreater<2699>())
	{
		// now it's completely obfuscated, so just ignoring the entire list of checks
		hook::jump(hook::get_call(hook::get_pattern("41 BD 20 00 00 00 48 8B CE 41 3B FD", -13)), ReturnTrue);
	}
	else
	{
		// some stat check in 'is allowed to run network game'; possibly SP prolog
		hook::put<uint8_t>(hook::pattern("BA 87 03 00 00 E8 ? ? ? ? 84 C0 75 14").count(1).get(0).get<void>(12), 0xEB);

		// same func, this time 'have tunables downloaded'
		auto match = hook::pattern("80 B8 89 00 00 00 00 75 14 48").count(1).get(0);

		hook::put<uint8_t>(match.get<void>(7), 0xEB);

		// and similarly, 'have bgscripts downloaded'
		hook::put<uint8_t>(match.get<void>(43), 0xEB);
	}

	// unknownland
	hook::put<uint16_t>(hook::pattern("8B B5 ? ? 00 00 85 F6 0F 84 ? 00 00").count(1).get(0).get<void>(8), 0xE990);

	// objectmgr bandwidth stuff?
	hook::put<uint8_t>(hook::pattern("F6 82 ? 00 00 00 01 74 2C 48").count(1).get(0).get<void>(7), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 21 80 7F ? FF B3 01 74 19 0F").count(1).get(0).get<void>(8), 0xEB);

	// even more stuff in the above function?!
	hook::nop(hook::pattern("85 ED 78 52 84 C0 74 4E 48").count(1).get(0).get<void>(), 8);

	// DLC mounts
	auto location = hook::pattern("0F 85 A4 00 00 00 8D 57 10 48 8D 0D").count(1).get(0).get<char>(12);

	g_dlcMountCount = (uint16_t*)(location + *(int32_t*)location + 4 + 8);

	// netgame state - includes connecting state
	location = hook::get_pattern<char>("48 83 EC 20 80 3D ? ? ? ? 00 48 8B D9 74 5F", 6);

	g_isNetGame = (bool*)(location + *(int32_t*)location + 4 + 1); // 1 as end of instruction is after '00', cmp

	// ignore CMsgJoinRequest failure reason '7' (seemingly related to tunables not matching?)
	hook::put<uint8_t>(hook::pattern("84 C0 75 0B 41 BC 07 00 00 00").count(1).get(0).get<void>(2), 0xEB);

	// also ignore the rarer CMsgJoinRequest failure reason '13' (something related to what seems to be like stats)
	hook::put<uint8_t>(hook::pattern("3B ? 74 0B 41 BC 0D 00 00 00").count(1).get(0).get<void>(2), 0xEB);

	// ignore CMsgJoinRequest failure reason 15 ('mismatching network timeout')
	hook::put<uint8_t>(hook::get_pattern("74 0B 41 BC 0F 00 00 00 E9", 0), 0xEB);

	// don't wait for shut down of NetRelay thread
	// not a thing in 2372 anymore
	if (!xbr::IsGameBuildOrGreater<2372>())
	{
		hook::return_function(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 83 3D ? ? ? ? FF 74", -16));
	}

	// don't redundantly switch clipset manager to network mode
	// (blocks on a LoadAllObjectsNow after scene has initialized already)
	g_clipsetManager_networkState =
		hook::get_address<int*>(hook::get_pattern("0F 85 6B FF FF FF C7 05", 6), 2, 10);
	MH_CreateHook(hook::get_pattern("83 E1 02 74 34 A8 04 75", -0x36),
		fwClipSetManager_StartNetworkSessionHook, (void**)&g_orig_fwClipSetManager_StartNetworkSession);

	// don't switch to SP mode either
	hook::return_function(hook::get_pattern("48 8D 2D ? ? ? ? 8B F0 85 C0 0F", -0x15));

	// start clipset manager off in network mode
	hook::put<uint32_t>(hook::get_pattern("0F 85 6B FF FF FF C7 05", 12), 2); // network state flag
	hook::put<uint8_t>(hook::get_pattern("F6 44 07 04 02 74 7A", 4), 4); // check persistent sp flag -> persistent mp

	// exitprocess -> terminateprocess
	MH_CreateHookApi(L"kernel32.dll", "ExitProcess", ExitProcessReplacement, nullptr);
	MH_EnableHook(MH_ALL_HOOKS);

	// nullify RageNetSend thread
	hook::put<uint16_t>(hook::get_pattern("41 BC 88 13 00 00 E8 ? ? ? ? 83 C8 01", -6), 0xE990);

	// nullify RageNetRecv thread
	if (xbr::IsGameBuildOrGreater<2372>())
	{
		hook::nop(hook::get_pattern("45 38 AE 80 00 00 00 0F 84", 7), 6);
	}
	else
	{
		hook::nop(hook::get_pattern("41 F6 47 40 02 0F 84 ? ? ? ? 49 8B 4F 28 BA", 5), 6);
	}

	// get calls for RageNetSend function
	hook::set_call(&g_handleQueuedSend, hook::get_pattern("48 8B CE E8 ? ? ? ? 48 8D BE ? ? 00 00 41", 3));

	// replace the call to thread init to get the internal connection manager struct address
	{
		void* callOff = hook::get_pattern("80 8B ? ? ? ? 04 48 8D 8B ? ? ? ? 48 8B", 17);
		void* func = (xbr::IsGameBuildOrGreater<2372>()) ? (void*)&CustomCreateSendThreads<2372> : &CustomCreateSendThreads<1604>;

		hook::set_call(&g_origCreateSendThreads, callOff);
		hook::call(callOff, func);
	}

	// netrelay thread handle calls
	if (!xbr::IsGameBuildOrGreater<2372>())
	{
		char* location = hook::get_pattern<char>("48 8D 8D C8 09 00 00 41 B8", 13);
		hook::set_call(&g_receivePacket, location);
		hook::set_call(&g_handlePacket, location + 22);
	}

	// change session count
	// 1604 changed this address to be a bit more specific
	hook::put<uint32_t>(hook::get_pattern("C7 87 ? ? ? 00 18 00 00 00 C7 87", 6), 0x40 >> 1);

	// add a OnMainGameFrame to do net stuff
	OnMainGameFrame.Connect([]()
	{
		RunNetworkStuff();
	});

	// disable unknown stuff
	{
		// 1032/1103!
		if (!xbr::IsGameBuildOrGreater<2060>())
		{
			// 1868 integrity checks this
			hook::return_function(hook::get_pattern("44 8B 99 08 E0 00 00 4C 8B C9 B9 00 04", 0));
		}
	}

	// async SC init
	{
		auto location = hook::get_pattern<char>("E8 ? ? ? ? 84 C0 75 B6 88 05");
		hook::set_call(&_isScWaitingForInit, location);
		hook::call(location, ReturnFalse);

		SetScInitWaitCallback(_isScWaitingForInit);

		void(*_processEntitlements)();
		hook::set_call(&_processEntitlements, location - 50);

		OnLookAliveFrame.Connect([_processEntitlements]()
		{
			_processEntitlements();

			if (!Instance<ICoreGameInit>::Get()->GetGameLoaded())
			{
				g_netLibrary->RunMainFrame();
			}
		});
	}

	// pretend to have CGameScriptHandlerNetComponent always be host
	// (we now use 'real' network scripts with net component, and compatibility
	// mandates check of netComponent && netComponent->IsHost() always fails)
	hook::jump(hook::get_pattern("33 DB 48 85 C0 74 17 48 8B 48 10 48 85 C9 74 0E", -10), ReturnTrue);

	// don't consider ourselves as host for world state reassignment
	hook::put<uint16_t>(hook::get_pattern("EB 02 32 C0 84 C0 0F 84 B4 00", 6), 0xE990);

	// network host tweaks
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		auto origCreatePickup = fx::ScriptEngine::GetNativeHandler(0x891804727E0A98B7);

		// CPickupPlacement needs flag `1` to actually work without net component
		fx::ScriptEngine::RegisterNativeHandler(0x891804727E0A98B7, [=](fx::ScriptContext& context)
		{
			context.SetArgument(7, context.GetArgument<int>(7) | 1);

			(*origCreatePickup)(context);
		});
	});

	// always return 'true' from netObjectMgr duplicate script object ID checks
	// (this function deletes network objects considered as duplicate)
	hook::jump(hook::get_pattern("49 8B 0C 24 0F B6 51", -0x69), ReturnTrue);

	// 1365 requirement: wait for SC to have inited before loading vehicle metadata
	{
		auto location = hook::get_pattern("BA 49 00 00 00 E8 ? ? ? ? EB 28", 0x20);

		hook::set_call(&_origLoadMeta, location);
		hook::call(location, WaitForScAndLoadMeta);
	}

	// replay editor unload wait
	{
		auto location = hook::get_pattern("0F 85 ? ? ? ? 33 DB 38 1D ? ? ? ? 75", -0x14);
		MH_CreateHook(location, BeforeReplayLoadHook, (void**)&g_origBeforeReplayLoad);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// don't stop police scanner reports when changing time in a networked game (really?)
	{
		auto location = hook::get_pattern("48 8D 0D ? ? ? ? 33 D2 E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 48 08", 9);
		hook::set_call(&g_origPoliceScanner_Stop, location);
		hook::call(location, PoliceScanner_StopWrap);
	}

	// default netnoupnp and netnopcp to true
	auto netNoUpnp = hook::get_address<int*>(hook::get_pattern("8A D1 76 02 B2 01 48 39 0D", 9));
	auto netNoPcp = hook::get_address<int*>(hook::get_pattern("8A D1 EB 02 B2 01 48 39 0D", 9));

	OnGameFrame.Connect([netNoPcp, netNoUpnp]()
	{
		*netNoUpnp = TRUE;
		*netNoPcp = TRUE;
	});
});
