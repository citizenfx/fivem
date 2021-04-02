#include <StdInc.h>
#include <Hooking.h>
#include <Streaming.h>

static void* g_storeMgr;

static hook::cdecl_stub<void(bool)> g_loadObjectsNow([]()
{
	return hook::get_call(hook::get_pattern("03 CB 8B 4C C8 04 80 E1 03 80 F9 01 75", -0x12));
});

static hook::cdecl_stub<void(void*, uint32_t, int)> g_requestObject([]()
{
	return hook::get_call(hook::get_pattern("45 8B CD 41 B8 03 00 00 00 E8 ? ? ? ? 33 D2 48", 9));
});

static hook::cdecl_stub<bool(void*, uint32_t)> g_releaseObject([]()
{
	return hook::get_call(hook::get_pattern("41 03 D0 45 33 C0 E8 ? ? ? ? 48 85 FF 74", 6));
});

static hook::cdecl_stub<bool(void*, uint32_t, int)> g_releaseSystemObject([]()
{
	return hook::get_pattern("66 23 C6 0F B7 D0 66 41 89 54 D8 06", -0x43);
});

static hook::cdecl_stub<streaming::strStreamingModule**(void*, uint32_t)> g_getStreamingModule([]()
{
	return hook::get_pattern("44 0F B7 51 10 45 33 C9 4C 8B 59 08", 0);
});

static hook::cdecl_stub<int(const char*)> g_getStreamingExtIndex([]()
{
	return hook::get_pattern("41 0F B6 09 43 0F B6 3C 19 2B CF", -43);
});

static hook::cdecl_stub<streaming::strStreamingModule*(void*, int)> g_getStreamingModuleFromExtIndex([]()
{
	return hook::get_call(hook::get_pattern("48 85 FF 74 ? 48 8D 4F 01", 0x17));
});

static hook::cdecl_stub<uint32_t*(uint32_t*, const char*, bool, const char*, bool, bool)> g_registerRawStreamingFile([]()
{
	return hook::get_pattern("48 8B CE B2 01 4D 8B F9 45 8A E0 E8", -0x26);
});

static hook::cdecl_stub<size_t(StreamingDataEntry*, uint32_t, void*, bool)> _computeVirtualSize([]()
{
	return hook::get_call(hook::get_pattern("41 8B 57 ? 45 33 C9 45 33 C0 48 8B CF E8", 13));
});

static hook::cdecl_stub<size_t(StreamingDataEntry*, uint32_t)> _computePhysicalSize([]()
{
	return hook::get_call(hook::get_pattern("41 8B 57 ? 45 33 C9 45 33 C0 48 8B CF E8", 34));
});

static hook::cdecl_stub<bool(streaming::Manager*, uint32_t, int)> _isReadyToDelete([]()
{
	return hook::get_pattern("8B DA 44 8B 4C D8 04 41 8B C1", -0x10);
});

static hook::cdecl_stub<void(streaming::Manager*, atArray<uint32_t>&, uint32_t)> _getDependents([]()
{
	return hook::get_pattern("4C 8B F9 0F 86 ? ? ? ? 44 8D 63 01 41 BD", -0x28);
});

size_t StreamingDataEntry::ComputePhysicalSize(uint32_t strIndex)
{
	return _computePhysicalSize(this, strIndex);
}

size_t StreamingDataEntry::ComputeVirtualSize(uint32_t strIndex, void* a3, bool a4)
{
	return _computeVirtualSize(this, strIndex, a3, a4);
}

static rage::strStreamingAllocator* g_streamingAllocator;

namespace rage
{
	strStreamingAllocator* strStreamingAllocator::GetInstance()
	{
		return g_streamingAllocator;
	}
}

namespace streaming
{
	bool Manager::IsObjectReadyToDelete(uint32_t streamingIndex, int flags)
	{
		return _isReadyToDelete(this, streamingIndex, flags);
	}

	void Manager::FindAllDependents(atArray<uint32_t>& outIndices, uint32_t objectId)
	{
		return _getDependents(this, outIndices, objectId);
	}

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
		auto index = g_getStreamingExtIndex(extension);
		auto module = g_getStreamingModuleFromExtIndex(this, index);
		return module;
	}

	Manager* Manager::GetInstance()
	{
		return (Manager*)g_storeMgr;
	}

	uint32_t* RegisterRawStreamingFile(uint32_t* fileId, const char* fileName, bool unkTrue, const char* registerAs, bool errorIfFailed)
	{
		return g_registerRawStreamingFile(fileId, fileName, unkTrue, registerAs, errorIfFailed, true);
	}
}

static HookFunction hookFunction([]()
{
	{
		g_storeMgr = hook::get_address<decltype(g_storeMgr)>(hook::get_pattern<char>("83 F8 FF 74 ? 8B 15 ? ? ? ? 48 8D 0D ? ? ? ? 03 D0", 14));
	}

	{
		g_streamingAllocator = hook::get_address<decltype(g_streamingAllocator)>(hook::get_pattern<char>("48 8D 14 76 48 8B 54 D5 BF 48 8D 0D", 12));
	}
});
