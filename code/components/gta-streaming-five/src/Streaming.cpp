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

static hook::cdecl_stub<bool(void*, uint32_t)> g_releaseObject([]()
{
	return hook::get_call(hook::get_pattern("45 33 C0 03 D7 E8 ? ? ? ? 48 8B 03 48 8B CB", 5));
});

static hook::cdecl_stub<bool(void*, uint32_t, int)> g_releaseSystemObject([]()
{
	return hook::get_pattern("8B CA 4D 8B 11 45 0F B7 5C CA 06 45", -0xB);
});

static hook::cdecl_stub<streaming::strStreamingModule**(void*, uint32_t)> g_getStreamingModule([]()
{
	return hook::get_pattern("45 33 C0 41 FF C9 41 8B C1 D1 F8 48", -0xD);
});

static hook::cdecl_stub<streaming::strStreamingModule*(void*, const char*)> g_getStreamingModuleFromExt([]()
{
	return hook::get_pattern("46 0F B7 0C 42 BA FF FF 00 00 66 44 3B CA 74 1D", -0x30);
});

static hook::cdecl_stub<uint32_t(uint32_t*, const char*, bool, const char*, bool)> g_registerRawStreamingFile([]()
{
	return hook::get_pattern("B2 01 48 8B CD 45 8A E0 4D 0F 45 F9 E8", -0x25);
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

	bool Manager::ReleaseObject(uint32_t objectId)
	{
		return g_releaseObject(this, objectId);
	}

	bool Manager::ReleaseObject(uint32_t objectId, int flags)
	{
		return g_releaseSystemObject(this, objectId, flags);
	}

	strStreamingModule* strStreamingModuleMgr::GetStreamingModule(int index)
	{
		return *g_getStreamingModule(this, index);
	}

	strStreamingModule* strStreamingModuleMgr::GetStreamingModule(const char* extension)
	{
		return g_getStreamingModuleFromExt(this, extension);
	}

	Manager* Manager::GetInstance()
	{
		return (Manager*)g_storeMgr;
	}

	uint32_t RegisterRawStreamingFile(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed)
	{
		return g_registerRawStreamingFile(fileId, fileName, unkTrue, registerAs, errorIfFailed);
	}
}

static HookFunction hookFunction([] ()
{
	{
		auto location = hook::get_pattern<char>("74 1A 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? 41", 11);

		g_storeMgr = (void*)(location + *(int32_t*)location + 4);
	}
});
