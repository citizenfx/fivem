#include <StdInc.h>
#include <Hooking.h>

#include <netObjectMgr.h>

static rage::netObjectMgr** g_objectMgr;

static hook::cdecl_stub<rage::netObject*(rage::netObjectMgr*, uint16_t, bool)> _getNetworkObject([]()
{
	return hook::get_pattern("44 38 B1 08 27 00 00 0F 84", -0x23);
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
	// 1737: arxan!!
	// 2060: arxan!!!!
	g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("2B C3 3D 88 13 00 00 0F 82 ? ? ? ? 48 8B 05", 16));
});
