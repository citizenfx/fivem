#include <StdInc.h>
#include <Hooking.h>
#include <Streaming.h>

static void* g_storeMgr;

static hook::cdecl_stub<void(bool)> g_loadObjectsNow([]()
{
	return hook::get_call(hook::get_pattern("41 B8 14 00 00 00 03 D3 E8", 0xF));
});

static hook::cdecl_stub<void(void*, uint32_t, int)> g_requestObject([]()
{
	return hook::get_call(hook::get_pattern("41 B8 14 00 00 00 03 D3 E8", 8));
});

static hook::cdecl_stub<void(void*, uint32_t)> g_releaseObject([]()
{
	return hook::get_call(hook::get_pattern("45 33 C0 03 D7 E8 ? ? ? ? 48 8B 03 48 8B CB", 5));
});

namespace streaming
{
	void LoadObjectsNow(bool a1)
	{
		g_loadObjectsNow(a1);
	}

	void Manager::RequestObject(uint32_t objectId, int flags)
	{
		g_requestObject(this, objectId, flags);
	}

	void Manager::ReleaseObject(uint32_t objectId)
	{
		g_releaseObject(this, objectId);
	}

	Manager* Manager::GetInstance()
	{
		return (Manager*)g_storeMgr;
	}
}

static HookFunction hookFunction([] ()
{
	{
		auto location = hook::get_pattern<char>("74 1A 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? 41", 11);

		g_storeMgr = (void*)(location + *(int32_t*)location + 4);
	}
});