#include <StdInc.h>
#include <Hooking.h>

#include <netObjectMgr.h>

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

static HookFunction hookFunction([]()
{
	g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("45 0F 57 C0 48 8B 35 ? ? ? ? 0F 57 FF", 7));
});
