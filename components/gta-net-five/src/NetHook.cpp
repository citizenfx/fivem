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

static NetLibrary* g_netLibrary;

#include <ws2tcpip.h>

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
			memcpy(buf, buffer, min(len, length));

			sockaddr_in* outFrom = (sockaddr_in*)from;
			memset(outFrom, 0, sizeof(sockaddr_in));

			outFrom->sin_family = AF_INET;
			outFrom->sin_addr.s_addr = ntohl((netID ^ 0xFEED) | 0xC0A80000);
			outFrom->sin_port = htons(6672);

			char addr[60];
			inet_ntop(AF_INET, &outFrom->sin_addr.s_addr, addr, sizeof(addr));

			*fromlen = sizeof(sockaddr_in);

			lastReceivedFrom = netID;

			trace("CfxRecvFrom (from %i %s) %i bytes on %p\n", netID, addr, length, s);

			return min(len, length);
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

			trace("CfxSendTo (to internal address %i) %i b (from thread 0x%x)\n", (htonl(toIn->sin_addr.s_addr) & 0xFFFF) ^ 0xFEED, len, GetCurrentThreadId());
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

int __stdcall CfxSelect(_In_ int nfds, _Inout_opt_ fd_set FAR *readfds, _Inout_opt_ fd_set FAR *writefds, _Inout_opt_ fd_set FAR *exceptfds, _In_opt_ const struct timeval FAR *timeout)
{
	bool shouldAddSocket = false;

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

bool GetOurSystemKey(char* systemKey);

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <strsafe.h>

struct ScInAddr
{
	uint64_t unkKey1;
	uint64_t unkKey2;
	uint32_t ipLan;
	uint16_t portLan;
	uint32_t ipUnk;
	uint16_t portUnk;
	uint32_t ipOnline;
	uint16_t portOnline;
	uint16_t pad3;
};

struct ScSessionAddr
{
	uint8_t sessionId[16];
	ScInAddr addr;
};

struct ScUnkAddr
{
	ScInAddr lanAddr; // ???
	uint64_t pad;
	ScSessionAddr addr;
	uint32_t unkVal;
	char pad2[292]; // seriously this struct is weird
	char systemKey[16];
};

static_assert(sizeof(ScInAddr) == (40), "ScInAddr size seems bad...");
static_assert(sizeof(ScSessionAddr) == (56), "ScSessionAddr size seems bad...");

bool StartLookUpInAddr(void*, void*, void* us, int* unkInt, bool something, int* a, ScSessionAddr* in, void*, ScUnkAddr* out, int* outSuccess, int* outStatus) // out might be the one before, or even same as in, dunno
{
	uint64_t sysKey = in->addr.unkKey1 ^ 0xFEAFEDE;

	memcpy(out->systemKey, in->sessionId, sizeof(out->systemKey));
	*(uint64_t*)(&out->systemKey[8]) = 0;

	out->pad = sysKey;
	out->lanAddr = in->addr;
	out->addr = *in;
	out->unkVal = 1;
	*outStatus = 3;
	*outSuccess = 1;

	return true;
}

static void(*g_origMigrateCopy)(void*, void*);

void MigrateSessionCopy(char* target, char* source)
{
	g_origMigrateCopy(target, source);

	ScSessionAddr* sessionAddress = reinterpret_cast<ScSessionAddr*>(target - 16);
	
	std::unique_ptr<NetBuffer> msgBuffer(new NetBuffer(64));

	msgBuffer->Write<uint32_t>((sessionAddress->addr.ipLan & 0xFFFF) ^ 0xFEED);
	msgBuffer->Write<uint32_t>(sessionAddress->addr.unkKey1);

	g_netLibrary->SendReliableCommand("msgHeHost", msgBuffer->GetBuffer(), msgBuffer->GetCurLength());
}

static hook::cdecl_stub<bool()> isNetworkHost([] ()
{
	return hook::get_call(hook::pattern("74 50 E8 ? ? ? ? 84 C0 75 10 48").count(1).get(0).get<void>(2));
});

static bool* didPresenceStuff;

static hook::cdecl_stub<void()> doPresenceStuff([] ()
{
	return hook::pattern("32 DB 38 1D ? ? ? ? 75 24 E8").count(1).get(0).get<void>(-6);
});

static hook::cdecl_stub<void(void*, ScSessionAddr*, int64_t, int)> joinGame([] ()
{
	return hook::pattern("F6 81 ? 82 00 00 01 45 8B F9 45 8B E8 4C 8B").count(1).get(0).get<void>(-0x24);
});

static hook::cdecl_stub<void(int, int, int)> hostGame([] ()
{
	return hook::get_call(hook::pattern("48 8B 41 10 BA 01 00 00 00 41 B8 05 01 00 00").count(1).get(0).get<void>(0x11));
});

static void* getNetworkManager()
{
	static void** networkMgrPtr = nullptr;

	if (networkMgrPtr == nullptr)
	{
		char* func = (char*)hook::get_call(hook::pattern("74 50 E8 ? ? ? ? 84 C0 75 10 48").count(1).get(0).get<void>(2));
		func += 9;

		networkMgrPtr = (void**)(func + *(int32_t*)func + 4);
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
	g_onlineAddress->ip1 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	g_onlineAddress->port1 = 6672;

	g_onlineAddress->ip2 = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	g_onlineAddress->port2 = 6672;

	return g_onlineAddress;
}

static uint16_t* g_dlcMountCount;

static HookFunction initFunction([] ()
{
	g_netLibrary = NetLibrary::Create();

	static NetLibrary* netLibrary;

	static bool doTickNextFrame;

	g_netLibrary->OnConnectOKReceived.Connect([] (NetAddress)
	{
		// redo presence stuff?
		*didPresenceStuff = false;

		doPresenceStuff();

		doTickNextFrame = true;
	});

	g_netLibrary->OnBuildMessage.Connect([] (NetBuffer& msg)
	{
		// hostie
		if (isNetworkHost())
		{
			msg.Write(0xB3EA30DE); // msgIHost
			msg.Write(g_netLibrary->GetServerBase());

			static bool doneSound;

			if (!doneSound)
			{
				PlaySound(MAKEINTRESOURCE(SND_ALIAS_SYSTEMHAND), nullptr, SND_ALIAS_ID | SND_ASYNC);

				doneSound = true;
			}
		}
	});

	g_netLibrary->OnInitReceived.Connect([] (NetAddress)
	{
		g_netLibrary->DownloadsComplete();
	});

	g_netLibrary->OnConnectionError.Connect([] (const char* e)
	{
		GlobalError("%s", e);
	});

	g_netLibrary->SetBase(GetTickCount());

	static bool doTickThisFrame = false;

	OnGameFrame.Connect([] ()
	{
		GetOurOnlineAddressRaw();

		g_netLibrary->RunFrame();

		/*static bool isNet = false;

		if (!isNet)
		{
			*(uint8_t*)0x189203D = 1;

			// related to above?
			//*(uint8_t*)0x13B4F50 = 1;
			//((void(*)())0x4E1A20)();

			((void(*)())0xC23CD0)();

			isNet = true;
		}*/

		// TODO: replace this so that reloading can work correctly
		static bool gameLoaded = false;

		Instance<ICoreGameInit>::Get()->OnGameFinalizeLoad.Connect([] ()
		{
			gameLoaded = true;
		});

		if (gameLoaded && doTickThisFrame)
		{
			gameLoaded = false;

			if (*g_dlcMountCount != 39)
			{
				GlobalError("DLC count mismatch - %d DLC mounts exist locally, but %d are expected. Please check that you have installed all core game updates and try again.", *g_dlcMountCount, 39);

				return;
			}

			if (g_netLibrary->GetHostNetID() == 0xFFFF || g_netLibrary->GetHostNetID() == g_netLibrary->GetServerNetID())
			//if (false)
			{
				/*GameInit::LoadGameFirstLaunch([] ()
				{
					return true;
				});*/

				// a2: player count
				// a3 & 1: private session
				// a3 & 2: something private-related
				// a3 & 4: single-player network session?
				// a3 & 16: friend-only session
				// a3 & 32: crew-only session?
				//hostGame(3, 8, 0x108);
				hostGame(0, 32, 0x0);
			}
			else
			{
				static ScSessionAddr netAddr;
				memset(&netAddr, 0, sizeof(netAddr));

				netAddr.addr.unkKey1 = g_netLibrary->GetHostBase();
				netAddr.addr.unkKey2 = g_netLibrary->GetHostBase();

				netAddr.addr.ipLan = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.portLan = 6672;

				netAddr.addr.ipUnk = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.portUnk = 6672;

				netAddr.addr.ipOnline = (g_netLibrary->GetHostNetID() ^ 0xFEED) | 0xc0a80000;
				netAddr.addr.portOnline = 6672;

				*(uint32_t*)&netAddr.sessionId[0] = 0x2;//g_netLibrary->GetHostBase() ^ 0xFEAFEDE;
				*(uint32_t*)&netAddr.sessionId[8] = 0xCDCDCDCD;

				//*(uint32_t*)&netAddr.addr.hostKey[0] = g_netLibrary->GetHostBase() ^ 0xFEAFEDE;
				//*(uint32_t*)&netAddr.addr.hostKey[4] = 0xCDCDCDCD;
				//*(uint64_t*)&netAddr.addr.hostKey[0] = 0;
				//*(uint64_t*)&netAddr.addr.hostKey[8] = 0;
				//*(uint16_t*)&netAddr.addr.hostKey[4] = g_netLibrary->GetHostNetID() ^ 0xFEED;

				//joinGame(getNetworkManager(), &netAddr, 1, 0);
				joinGame(getNetworkManager(), &netAddr, 1, 1);
			}

			doTickThisFrame = false;
		}

		if (doTickNextFrame)
		{
			doTickThisFrame = true;

			doTickNextFrame = false;
		}

		static int lastId;

		/*if (lastId != g_netLibrary->GetServerNetID())
		{
			((void(*)(int))0xF3F400)(0);

			lastId = g_netLibrary->GetServerNetID();
		}*/
	});
});

static uint64_t* g_globalNetSecurityKey;

static void GetOurSecurityKey(uint64_t* key)
{
	key[0] = 2;
	key[1] = 2;
	//key[0] = g_netLibrary->GetServerBase();
	//key[1] = 0;
}

bool GetOurOnlineAddress(ScInAddr* address)
{
	memset(address, 0, 24 + 16);
	address->unkKey1 = g_netLibrary->GetServerBase();
	address->unkKey2 = g_netLibrary->GetServerBase();
	address->ipLan = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->portLan = 6672;
	address->ipUnk = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->portUnk = 6672;
	address->ipOnline = (g_netLibrary->GetServerNetID() ^ 0xFEED) | 0xc0a80000;
	address->portOnline = 6672;
	//address->pad5 = 0x19;

	g_globalNetSecurityKey[0] = g_netLibrary->GetServerBase();
	g_globalNetSecurityKey[1] = g_netLibrary->GetServerBase();

	return true;
}

bool GetOurSystemKey(char* systemKey)
{
	// then set this stuff
	memset(systemKey, 0, 8);
	*(uint32_t*)&systemKey[0] = g_netLibrary->GetServerBase() ^ 0xFEAFEDE;
	//*(uint16_t*)&systemKey[4] = 12;// g_netLibrary->GetServerNetID() ^ 0xFEED;

	return true;
}

bool GetOurSessionKey(char* sessionKey)
{
	*(uint64_t*)sessionKey = 2;

	return true;
}

struct AutoIdDescriptor
{
	uintptr_t vtable;
	int unk;
	int id;
	int id2;
	const char* name;
	AutoIdDescriptor* parent;
	AutoIdDescriptor* next;
};

static AutoIdDescriptor** g_netMessage;

static void __stdcall LogDescriptorDo(int netType)
{
	if (g_netMessage)
	{
		for (AutoIdDescriptor* descriptor = (*g_netMessage)->next; descriptor->next; descriptor = descriptor->next)
		{
			if (descriptor->id == netType)
			{
				g_pendSendVar++;

				if (g_pendSendVar > 40)
				{
					// help! too many pending packets!
					__debugbreak();
				}

				trace("[%s] %d\n", descriptor->name, netType);
				return;
			}
		}
	}
}

#include <jitasm.h>

struct ItemHook : public jitasm::Frontend
{
public:
	virtual void InternalMain() override
	{
		push(rcx);
		push(rdx);

		sub(rsp, 32);

		mov(rax, (uintptr_t)LogDescriptorDo);
		call(rax);

		add(rsp, 32);

		pop(rdx);
		pop(rcx);

		mov(qword_ptr[rsp + 8 + 8], rbx);

		ret();
	}
};

static ItemHook itemHook;

static void(*origNetFrame)(void*, void*);

static void CustomNetFrame(void* a, void* b)
{
	//OnGameFrame();

	origNetFrame(a, b);
}

static int ReturnTrue()
{
	return true;
}

static bool* g_isNetworkGame;

static void DummySetNetworkGame()
{
	*g_isNetworkGame = true;
}

static void ByeWorld()
{
	__debugbreak();
}

struct CNonPhysicalPlayerData
{
	void* vtable;
	uint32_t bubbleId;
	uint32_t playerId;
	float positionX;
	float positionY;
	float positionZ;
};

static void(*g_origPhysical)(CNonPhysicalPlayerData*, uintptr_t);

void CustomPhysical(CNonPhysicalPlayerData* data, uintptr_t a2)
{
	//trace("-- CNonPhysicalPlayerData --\nBubble ID: %d\nPlayer ID: %d\nPosition: %f, %f, %f\n", data->bubbleId, data->playerId, data->positionX, data->positionY, data->positionZ);

	g_origPhysical(data, a2);
}

static bool ReturnTrueAndKillThatTask(char* playerObj)
{
	char* ped = *(char**)(playerObj + 80);
	char* task = *(char**)(ped + 32); // why is this in fwEntity anyway?

	task[308] = 0;

	return true;
}

void(*origSemaFunc)(void*, int);
void CustomSemaFunc(void* a, int b)
{
	//trace("signal sendto sema\n");

	origSemaFunc(a, b);
}

int(*origLogFunc)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);

int CustomLogFunc(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5)
{
	int retval = origLogFunc(a1, a2, a3, a4, a5);

	//trace("unk sendto func (ret %d)\n", retval);

	return retval;
}

int(*origFrag)(intptr_t, intptr_t, intptr_t, intptr_t);

int CustomFrag(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4)
{
	LARGE_INTEGER initTime;
	QueryPerformanceCounter(&initTime);

	int retval = origFrag(a1, a2, a3, a4);

	LARGE_INTEGER endTime;
	QueryPerformanceCounter(&endTime);

	static uint32_t lastTick = timeGetTime();

	uint32_t curTick = timeGetTime();

	if ((curTick - lastTick) > 250)
	{
		//trace("net send thread took long-ish (%i msec)\n", curTick - lastTick);
	}

	lastTick = curTick;

	if (retval)
	{
		//trace("got net packet on thread, retval %d, counts %d\n", retval, endTime.QuadPart - initTime.QuadPart);
	}

	return retval;
}

int(*origPerfSend)(intptr_t, intptr_t, intptr_t, intptr_t, intptr_t);

int CustomPerfSend(intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5)
{
	int retval = origPerfSend(a1, a2, a3, a4, a5);

	//trace("perf send (ret %d)\n", retval);

	return retval;
}

bool(*origCheckAddr)(intptr_t);

bool CustomCheckAddr(intptr_t a1)
{
	bool retval = origCheckAddr(a1);

	//trace("check addr (ret %d)\n", retval);

	return retval;
}

static int ReturnBandwidthCheck(char* base, int idx)
{
	int* bandwidth = (int*)(base + (96 * idx) + 436);

	//static int initBandwidth = *bandwidth;

	//*bandwidth = initBandwidth;
	//assert(bandwidth[1] == 0);
	//assert(*bandwidth >= 0);

	return 0;
}

static void(*g_origJR)(void*, void*, void*, void*);

void HandleJR(char* a1, char* a2, void* a3, void* a4)
{
	trace("snMsgJoinRequest - client's opinion: %16llx - server's opinion: %16llx\n", *(uint64_t*)a2, *(uint64_t*)(a1 + 3008 + 464));

	return g_origJR(a1, a2, a3, a4);
}

static HookFunction hookFunction([] ()
{
	/*OnPostFrontendRender.Connect([] ()
	{
		int value = *(int*)getNetworkManager();

		CRect rect(100, 100, 800, 800);
		CRGBA color(255, 255, 255, 255);

		TheFonts->DrawText(va(L"> %i <", value), rect, color, 100.0f, 1.0f, "Comic Sans MS");
	});*/

	void* handleJoinRequestPtr = hook::pattern("4C 8D 40 48 48 8D 95 D0 00 00 00 48 8B CE E8").count(1).get(0).get<void>(14);
	hook::set_call(&g_origJR, handleJoinRequestPtr);
	hook::call(handleJoinRequestPtr, HandleJR);

	char* fragPtr = hook::pattern("66 44 89 7C 24 34 66 44 89 7C 24 3C E8").count(1).get(0).get<char>(12);
	hook::set_call(&origFrag, fragPtr);
	hook::call(fragPtr, CustomFrag);

	fragPtr += 0xF7;

	char* perfSend = hook::get_call(fragPtr);

	hook::set_call(&origPerfSend, fragPtr);
	hook::call(fragPtr, CustomPerfSend);

	hook::set_call(&origCheckAddr, perfSend + 0x3F);
	hook::call(perfSend + 0x3F, CustomCheckAddr);

	char* location = hook::pattern("32 DB 38 1D ? ? ? ? 75 24 E8").count(1).get(0).get<char>(4);
	didPresenceStuff = (bool*)(location + *(int32_t*)location + 4);

	hook::iat("ws2_32.dll", CfxSendTo, 20);
	hook::iat("ws2_32.dll", CfxRecvFrom, 17);
	hook::iat("ws2_32.dll", CfxBind, 2);
	hook::iat("ws2_32.dll", CfxSelect, 18);
	hook::iat("ws2_32.dll", CfxGetSockName, 6);

	// session migration, some 'inline' memcpy of the new address
	void* migrateCmd = hook::pattern("48 8B 47 68 48 83 E9 80 48 89 41 F8 E8").count(1).get(0).get<void>(12);
	hook::set_call(&g_origMigrateCopy, migrateCmd);
	hook::call(migrateCmd, MigrateSessionCopy);

	// session key getting system key; replace with something static for migration purposes
	hook::call(hook::pattern("74 15 48 8D 4C 24 78 E8").count(1).get(0).get<void>(7), GetOurSessionKey);

	// we also need some pointers from this function
	char* netAddressFunc = hook::pattern("48 89 39 48 89 79 08 89 71 10 66 89 79 14 89 71").count(1).get(0).get<char>(-0x1E);
	hook::jump(netAddressFunc, GetOurOnlineAddress);

	char* onlineAddressFunc = hook::get_call(netAddressFunc + 0x75);

	netAddressFunc += 0x1A;

	bool* didNetAddressBool = (bool*)(netAddressFunc + *(int32_t*)netAddressFunc + 4);

	*didNetAddressBool = true;

	netAddressFunc += 0x25;

	g_globalNetSecurityKey = (uint64_t*)(netAddressFunc + *(int32_t*)netAddressFunc + 4);

	hook::jump(onlineAddressFunc, GetOurOnlineAddressRaw);

	onlineAddressFunc += 0x23;

	g_onlineAddress = (OnlineAddress*)(onlineAddressFunc + *(int32_t*)onlineAddressFunc + 4);

	//uint64_t* addrThing = (uint64_t*)(netAddressFunc + *(int32_t*)netAddressFunc + 4);
	//addrThing[0] = 1;
	//addrThing[1] = 1;

	// system key
	hook::jump(hook::pattern("48 83 F8 FF 75 17 48 8D 0D").count(1).get(0).get<void>(-32), GetOurSystemKey);

	// other system key thing
	hook::jump(hook::pattern("84 C0 74 0C 48 8B 44 24 38 48 89 03 B0").count(1).get(0).get<void>(-0x13), GetOurSystemKey);

	itemHook.Assemble();
	hook::call(hook::pattern("48 83 EC 20 48 8B DA BE 01 00 00 00 32 D2 8B F9").count(1).get(0).get<void>(-0xB), itemHook.GetCode());

	// network frame
	location = hook::pattern("C7 45 10 8B 30 7A FE E8 ? ? ? ? 4C 8D 45 10 48 8D 15 ? ? ? ? F3 0F 11").count(1).get(0).get<char>(19);

	uintptr_t addr = (uintptr_t)(location + *(int32_t*)location + 4);

	// patch 350 adds a 'xor ecx, ecx' before the jump
	if (*(char*)(addr) == 0x33)
	{
		addr += 2;
	}

	hook::set_call(&origNetFrame, addr);
	hook::jump(addr, CustomNetFrame);

	// network game unsetters
	/*location = hook::pattern("88 1D ? ? ? ? 84 C0 75 33 E8").count(1).get(0).get<char>(2);

	g_isNetworkGame = (bool*)(location + *(int32_t*)location + 4);
	//*isNetworkGame = true;

	location -= 2;

	hook::nop(location, 6);

	// #2
	hook::nop(hook::pattern("88 05 ? ? ? ? E9 82 00 00 00 48").count(1).get(0).get<void>(), 6);

	// #3
	hook::nop(hook::pattern("74 0C 40 88 35 ? ? ? ? E9 B3 00 00 00 48").count(1).get(0).get<void>(2), 7);

	// #4 in oddfuscated func
	location = hook::pattern("84 C0 0F 45 CD 88 0D ? ? ? ? 33 C9").count(1).get(0).get<char>(5);

	hook::nop(location, 6);
	hook::call(location, DummySetNetworkGame);*/

	// let's assume we're not an internet peer, therefore we don't try to encode anything
	//hook::jump(hook::pattern("66 44 39 41 04 74 24 8B 01 41 8D 50 01").count(1).get(0).get<void>(-3), ReturnTrue);

	// unknown obfuscated check
	hook::jump(hook::get_call(hook::pattern("84 C0 74 0F BA 03 00 00 00 48 8B CB E8 ? ? ? ? EB 0E").count(1).get(0).get<void>(12)), ReturnTrue);

	// locate address thingy
	//hook::call(hook::pattern("89 44 24 28 41 8B 87 80 01 00 00 48 8B CB 89 44").count(1).get(0).get<void>(18), StartLookUpInAddr);
	//hook::jump(hook::get_call(hook::pattern("48 8B D0 C7 44 24 28 04 00 00 00 44 89 7C 24 20").count(1).get(0).get<void>(16)), StartLookUpInAddr);
	hook::jump(hook::get_call(hook::pattern("45 33 C0 C6 44 24 28 01 44 89 7C 24 20").count(1).get(0).get<void>(13)), StartLookUpInAddr);

	// temp dbg: always clone a player (to see why this CTaskMove flag is being a twat)
	//hook::jump(hook::pattern("74 06 F6 40 2F 01 75 0E 48 8B D7").count(1).get(0).get<void>(-0x27), ReturnTrue);
	hook::jump(hook::pattern("74 06 F6 40 2F 01 75 0E 48 8B D7").count(1).get(0).get<void>(-0x27), ReturnTrueAndKillThatTask);
	
	// other security key thing
	//hook::jump(hook::pattern("8B C0 41 8B C9 48 69 C9 A7 FA DC 5C 48").count(1).get(0).get<void>(-0x38), GetOurSecurityKey);

	/*void* thing = hook::pattern("4C 8B F1 E8 ? ? ? ? 41 8B 8E E0 00 00 00 83").count(1).get(0).get<void>();
	void* udderThing = hook::pattern("48 8D A8 B8 FD FF FF 48 81 EC 10 03 00 00 4C 8B").count(1).get(0).get<void>();

	void* cmdThing = hook::pattern("4D 8B F0 48 8B FA 44 8D 7B 02 48 8B F1 48 39").count(1).get(0).get<void>();

	void* iceThing = hook::pattern("FF C0 85 C1 0F 84 CA 00 00 00 83 C8 FF 48 8D").count(1).get(0).get<void>();
	void* inFrameThing = hook::pattern("49 89 04 24 8B 45 08 49 8B CF 41 89 44 24 48").count(1).get(0).get<void>();

	void* otherInThing = hook::pattern("48 8D 45 D0 45 8B CE 4C 8B C7 49 8B D4 49 8B CF").count(1).get(0).get<void>();

	void* sendKeyThing = hook::pattern("41 F6 C4 01 74 2F 48").count(1).get(0).get<void>();

	void* recvKeyThing = hook::pattern("48 81 EC A0 02 00 00 45 33 E4 41 8B F9 4D 8B F0").count(1).get(0).get<void>();

	void* unkThingEarlier = hook::pattern("41 57 48 83 EC 60 33 DB  4D 8B F0 48 8B FA").count(1).get(0).get<void>();

	void* joinReq = hook::pattern("4C 8D 8D E0 08 00 00 48 8D 95 20 04 00 00 48 8D").count(1).get(0).get<void>();

	void* joinReqHandler = hook::pattern("49 8B 56 40 48 8D 8D D0 00 00 00 44 8B 42 58").count(2).get(0).get<void>(); 

	void* rlSessionCmdJoin = hook::pattern("48 8B D9 8B 89 F8 02 00 00 85 C9 0F 84 DC 00 00").count(1).get(0).get<void>();

	void* handleJoinResponse = hook::pattern("48 81 EC 80 08 00 00 49 8B 00 48 8B F1 49 8B C8").count(1).get(0).get<void>();

	void* handleJoinResponseTaskCompletion = hook::pattern("44 8D 73 02 4C 8B EA 48 8B F1 41 2B C6 83 F8 01").count(1).get(0).get<void>();

	void* getHostGamerData = hook::pattern("0F B7 43 0C 66 89 46 74 8B 44 24 60 89 46 7C 85").count(1).get(0).get<void>();

	void* migrate = hook::pattern("41 57 48 81 EC 50 06 00  00 48 8B F1 48").count(1).get(0).get<void>();

	void* setHostSessionId = hook::pattern("84 C0 75 D4 F6 83 ? ? ? ? 08").count(1).get(0).get<void>();

	void* startle = hook::pattern("48 83 A4 24 30 06 00 00 00 48 8D 94 24 30 06 00").count(1).get(0).get<void>();

	void* objectMgrPlayerToPlayerUpdate = hook::pattern("48 8B D9 84 D2 74 47 E8").count(1).get(0).get<void>();

	void* createScriptEntityExtension = hook::pattern("48 89 51 60 48 89 71 68 C6 41 70 0D 48 8D 4A 10").count(1).get(0).get<void>();

	void* unkEventHandler = hook::pattern("49 8B 00 48 8B CB FF 10  85 C0 0F 84").count(1).get(0).get<void>();

	void* bubbleFlag = hook::pattern("74 23 48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0").count(1).get(0).get<void>();

	void* guestStartComplete = hook::pattern("84 C0 74 4B 8B CE E8").count(1).get(0).get<void>();

	void* guestStartProc = hook::pattern("83 B9 38 0B 00 00 02 48 8B EA 48 8B D9 0F 85").count(1).get(0).get<void>();

	void* snEventStuff = hook::pattern("49 8B 00 48 8B CB FF 10 85 C0 0F 84").count(1).get(0).get<void>();

	void* sendBubbleThing = hook::pattern("48 8B D9 0F 84 C2 00 00 00 8A 49 2D E8").count(1).get(0).get<void>();

	void* pedToNetObj = hook::pattern("41 8A E8 44 0F B7 F2 74 35 E8").count(1).get(0).get<void>();

	void* addNetObjForPlayer = hook::pattern("44 8A 62 4B 33 DB 41 8B E9 4D 8B F8 48 8B F2 4C").count(1).get(0).get<void>();

	void* addNetObjGlobally = hook::pattern("41 56 48 83 EC 20 48 8B F2 0F B7 52 0A 41 B0 01").count(1).get(0).get<void>();	

	void* playerToPlayerInternal = hook::pattern("0F B6 47 2D 48 03 C0 4C 8B 6C C6 08 E9").count(1).get(0).get<void>();

	void* playerToPlayerDoSync = hook::pattern("8B 86 ? 7A 00 00 89 83 84 00 00 00 85 FF 0F 84").count(1).get(0).get<void>();

	void* markNetworkObjectForPlayer = hook::pattern("FF 90 F0 00 00 00 40 8A CF 84 C0 B8 01 00 00 00").count(1).get(0).get<void>();

	void* packCloneCreate = hook::pattern("4D 8B E1 4D 8B F8 FF 90  18 01 00 00 33 ED 84 C0").count(1).get(0).get<void>();

	void* aroundPlayerNetObject = hook::pattern("0F 94 C3 E8 ? ? ? ? 84 DB 74 13 45 38").count(1).get(0).get<void>();*/

	/*if (!_stricmp(getenv("COMPUTERNAME"), "fallarbor"))
	{
		hook::call(hook::pattern("0F 84 A2 00 00 00 48 8B D0 48 8B CE E8").count(1).get(0).get<void>(12), ByeWorld);

		hook::jump(hook::pattern("48 81 EC 20 06 00 00 48  8B FA 8B 12 48 8B D9").count(1).get(0).get<void>(-0xB), ByeWorld);

		hook::jump(hook::pattern("41 57 48 81 EC 50 06 00  00 48 8B F1 48").count(1).get(0).get<void>(-0x16), ByeWorld);
	}*/

	void* updateScAdvertisement = hook::pattern("48 89 44 24 20 E8 ? ? ? ? F6 D8 1B C9 83 C1").count(1).get(0).get<void>();

	// non-physical player data logging 'hack'
	location = hook::pattern("48 8B D0 48 85 C0 74 4E 48 8D 05").count(2).get(0).get<char>(11);

	{
		void** vtPtr = (void**)(location + *(int32_t*)location + 4);

		g_origPhysical = reinterpret_cast<decltype(g_origPhysical)>(vtPtr[2]);
		vtPtr[2] = CustomPhysical;
	}

	// temporary(!) patch to force CGameScriptObjInfo to act as if having an unknown identifier set (as regular creation doesn't set it?! - doesn't write to it at all)
	hook::nop(hook::pattern("83 79 10 00 74 05 48 8D 41 08 C3 33 C0 C3").count(1).get(0).get<void>(4), 2);

	// really weird patch to auto-start the session (?s in short jumps are because of +0x38 differences with steam/retail)
	//hook::put<uint8_t>(hook::pattern("84 C0 74 ? 83 BB ? ? 00 00 07 74 ? E8").count(1).get(0).get<void>(2), 0xEB);

	// some stat check in 'is allowed to run network game'; possibly SP prolog
	hook::put<uint8_t>(hook::pattern("BA 87 03 00 00 E8 ? ? ? ? 84 C0 75 14").count(1).get(0).get<void>(12), 0xEB);

	// same func, this time 'have tunables downloaded'
	auto match = hook::pattern("80 B8 89 00 00 00 00 75 14 48").count(1).get(0);

	hook::put<uint8_t>(match.get<void>(7), 0xEB);

	// and similarly, 'have bgscripts downloaded'
	hook::put<uint8_t>(match.get<void>(43), 0xEB);

	// unknownland
	hook::put<uint16_t>(hook::pattern("8B B5 A0 02 00 00 85 F6 0F 84 B1").count(1).get(0).get<void>(8), 0xE990);

	// always set the net sendto semaphore
	char* ptrT = hook::pattern("F7 84 24 80 00 00 00 00 00 00 01 74 23").count(1).get(0).get<char>(11);

	hook::nop(ptrT, 2); // direct condition
	hook::put<uint8_t>(hook::pattern("83 E5 01 75 13 8B 50 1C 49 8B CF").count(1).get(0).get<void>(3), 0xEB); // always bypass bandwidth checks
	hook::nop(hook::pattern("84 C0 75 32 85 ED 74 63 49 8B CF").count(1).get(0).get<void>(6), 2); // same as above
	hook::put<uint8_t>(hook::pattern("F6 84 24 80 00 00 00 01 75 02 B3 01").count(1).get(0).get<void>(8), 0xEB);

	ptrT += (0x110 - 0xF0);
	hook::set_call(&origSemaFunc, ptrT);
	hook::call(ptrT, CustomSemaFunc);

	ptrT -= (0x110 - 0xA7);
	hook::set_call(&origLogFunc, ptrT);
	hook::call(ptrT, CustomLogFunc);

	// objectmgr bandwidth stuff?
	hook::put<uint8_t>(hook::pattern("F6 82 98 00 00 00 01 74 2C 48").count(1).get(0).get<void>(7), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 21 80 7F 2D FF B3 01 74 19 0F").count(1).get(0).get<void>(8), 0xEB);

	// even more stuff in the above function?!
	hook::nop(hook::pattern("85 ED 78 52 84 C0 74 4E 48").count(1).get(0).get<void>(), 8);

	// bandwidth fubbling
	hook::nop(hook::pattern("2B 83 B8 09 00 00 3B 46 44 8A C2 77 03").count(1).get(0).get<void>(11), 2);

	// more bandwidth ignorance
	hook::jump(hook::pattern("48 83 EC 20 8B F2 48 8B D9 48 8D 3C 76 48 C1").count(1).get(0).get<void>(-0xB), ReturnBandwidthCheck);

	// don't subtract bandwidth either
	hook::return_function(hook::pattern("48 83 EC 20 48 83 79 38 00 41 8B F9 4D 8B F0 48").count(1).get(0).get<void>(-0x15));

	// or maybe it's not bandwidth-related at all, and it just needs this timer thing to not break
	// >it never breaks anyway, but yeah
	hook::put<uint8_t>(hook::pattern("83 BB 18 01 00 00 00 89 93 1C 01 00 00 7F 38").count(1).get(0).get<void>(13), 0xEB);

	// and just for kicks we'll remove this one as well
	hook::nop(hook::pattern("44 29 A3 10 01 00 00 83 BB 10 01 00 00 00 0F 8F").count(1).get(0).get<void>(14), 6);

	// DLC mounts
	location = hook::pattern("0F 85 A4 00 00 00 8D 57 10 48 8D 0D").count(1).get(0).get<char>(12);

	g_dlcMountCount = (uint16_t*)(location + *(int32_t*)location + 4 + 8);

	// find autoid descriptors
	auto matches = hook::pattern("48 89 03 8B 05 ? ? ? ? A8 01 75 21 83 C8 01 48 8D 0D");

	for (int i = 0; i < matches.size(); i++)
	{
		location = matches.get(i).get<char>(-4);
		void** vt = (void**)(location + *(int32_t*)location + 4);

		// RTTI locator
		vt--;

		struct RttiLocator
		{
			int signature;
			int offset;
			int cdOffset;
			uint32_t pTypeDescriptor;
		};

		RttiLocator* locator = (RttiLocator*)*vt;

		if (locator->signature == 1)
		{
			char* namePtr = (char*)(0x140000000 + locator->pTypeDescriptor + 16);

			if (strcmp(namePtr, ".?AV?$AutoIdDescriptor_T@VnetMessage@rage@@@rage@@") == 0)
			{
				location = matches.get(i).get<char>(19);

				g_netMessage = (AutoIdDescriptor**)(location + *(int32_t*)location + 4);

				break;
			}
		}
	}
});