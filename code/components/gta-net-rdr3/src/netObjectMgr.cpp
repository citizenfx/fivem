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

static HookFunction hookFunction([]()
{
	g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("45 0F 57 C0 48 8B 35 ? ? ? ? 0F 57 FF", 7));
	// Don't run function in onesync. (playerObjects is 32-sized and isn't given information in onesync)
	g_updateAllNetworkObjects = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 44 8B 35 ? ? ? ? 33 C0")), netObjectMgr__updateAllNetworkObjects);
});
