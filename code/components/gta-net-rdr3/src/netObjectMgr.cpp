#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <netObjectMgr.h>
#include <ICoreGameInit.h>

static rage::netObjectMgr** g_objectMgr;

static hook::cdecl_stub<rage::netObject* (rage::netObjectMgr*, uint16_t, bool)> _getNetworkObject([]()
{
	return hook::get_pattern("66 89 54 24 ? 56", -0xA);
});

namespace rage
{
netObjectMgr* netObjectMgr::GetInstance()
{
	return *g_objectMgr;
}

netObject* netObjectMgr::GetNetworkObject(uint16_t id, bool a3)
{
	return _getNetworkObject(this, id, a3);
}
}

extern ICoreGameInit* icgi;

static void (*g_updateAllNetworkObjects)(void*);
static void netObjectMgr__updateAllNetworkObjects(void* objMgr)
{
	if (icgi->OneSyncEnabled)
	{
		return;
	}

	g_updateAllNetworkObjects(objMgr);
}

static void (*g_updateAllRemoteObjects)(void*);
static void netObjectMgr__updateAllRemoteObjects(void* objMgr)
{
	if (icgi->OneSyncEnabled)
	{
		return;
	}

	g_updateAllRemoteObjects(objMgr);
}

static HookFunction hookFunction([]()
{
	g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("45 0F 57 C0 48 8B 35 ? ? ? ? 0F 57 FF", 7));
	// Don't run legacy update functions in onesync. (playerObjects is 32-sized and isn't given information in onesync)
	g_updateAllNetworkObjects = hook::trampoline(hook::get_pattern("4C 8B F9 89 BC 24", -34), netObjectMgr__updateAllNetworkObjects);
	g_updateAllRemoteObjects = hook::trampoline(hook::get_pattern("48 8B FA 48 8B F1 E8 ? ? ? ? 45 33 FF 44 89 BD", -42), netObjectMgr__updateAllRemoteObjects);
});
