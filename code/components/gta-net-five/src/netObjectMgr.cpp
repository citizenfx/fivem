#include <StdInc.h>
#include <Hooking.h>

#include <netObjectMgr.h>

static rage::netObjectMgr** g_objectMgr;

namespace rage
{
netObjectMgr* netObjectMgr::GetInstance()
{
	return *g_objectMgr;
}
}

static HookFunction hookFunction([]()
{
	g_objectMgr = hook::get_address<rage::netObjectMgr**>(hook::get_pattern("B9 C8 7F 00 00 E8 ? ? ? ? 48 85 C0", 0x30));
});
