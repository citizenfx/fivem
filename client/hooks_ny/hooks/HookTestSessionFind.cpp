// session finding hook for testing only

#include "StdInc.h"
#include "../live/Live.h"
#include "CrossLibraryInterfaces.h"

#pragma pack(push, 1)
struct SessionData
{
	char pad[40];
	uint32_t data;
	char pad2[301];
	XSESSION_INFO sessionInfo;
	char pad4[11];
} sessionData;
#pragma pack(pop)

void FindGameStuff(int, int, int, int)
{
	trace("finding games...\n");

	memset(&sessionData.sessionInfo, 0, sizeof(sessionData.sessionInfo));
	PXSESSION_INFO pSessionInfo = &sessionData.sessionInfo;

	uint16_t hostID = g_netLibrary->GetHostNetID();

	if (hostID != 65536)
	{
		pSessionInfo->sessionID.ab[0] = 1;
		pSessionInfo->keyExchangeKey.ab[0] = 1;

		pSessionInfo->hostAddress.ina.s_addr = hostID;
		pSessionInfo->hostAddress.inaOnline.s_addr = hostID;

		pSessionInfo->hostAddress.wPortOnline = g_netLibrary->GetHostBase() & 0xFFFF;
		*(uint64_t*)(pSessionInfo->hostAddress.abOnline) = g_netLibrary->GetHostBase() | (g_netLibrary->GetHostBase() << 32);

		sessionData.data = 1;

		*(DWORD*)0x18C7D4C = (DWORD)&sessionData;
		*(DWORD*)0x18C7D54 = 1;

		*(DWORD*)0x18C7D58 = 0; // pending 1
		*(DWORD*)0x18EDAB0 = 0; // pending 1
	}
}

DEFINE_INJECT_HOOK(networkFindGamesHook, 0x461850)
{
	return JumpTo((DWORD)FindGameStuff);
}

static HookFunction hookFunction([] ()
{
	hook::nop(0x462981, 10); // set_server_id overwrite stuff

	networkFindGamesHook.inject();

	// force lan session:
	hook::put<uint8_t>(0x1724426, 1); // lan session flag

	// setters:
	hook::nop(0x460179, 6);
	hook::nop(0x463ABB, 6);
	hook::nop(0xB604A4, 6);

	// test-icles to force non-host path for notifying a player of session members
	/*hook::put<uint8_t>(0x540380, 0x90);
	hook::put<uint8_t>(0x540381, 0xE9);

	hook::put<uint8_t>(0x54043F, 0xEB); // 'compare to host's value'*/
});