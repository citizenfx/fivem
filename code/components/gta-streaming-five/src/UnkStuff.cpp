#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <sysAllocator.h>

#include <CrossBuildRuntime.h>
#include <LaunchMode.h>

static int ReturnTrue()
{
	return 1;
}

#include <Streaming.h>
#include <VFSManager.h>

#include <atPool.h>
#include <MinHook.h>

#include <Error.h>

extern hook::cdecl_stub<rage::fiCollection*()> getRawStreamer;

#define VFS_GET_RCD_DEBUG_INFO 0x30001

struct GetRcdDebugInfoExtension
{
	const char* fileName; // in
	std::string outData; // out
};

static void ErrorInflateFailure(char* ioData, char* requestData, int zlibError, char* zlibStream)
{
	if (streaming::IsStreamerShuttingDown())
	{
		trace("Streamer shutdown: ignoring inflate() failure!\n");
		return;
	}

	uint32_t handle = *(uint32_t*)(requestData + 4);
	uint8_t* nextIn = *(uint8_t**)(ioData + 8);
	uint32_t availIn = *(uint32_t*)(ioData);
	uint32_t totalIn = *(uint32_t*)(ioData + 16);
	const char* msg = *(const char**)(zlibStream + 32);

	if (zlibError == -5 /* Z_BUF_ERROR */ && availIn == 0)
	{
		trace("Ignoring Z_BUF_ERROR with avail_in == 0.\n");
		return;
	}

	// get the entry name
	uint16_t fileIndex = (handle & 0xFFFF);
	uint16_t collectionIndex = (handle >> 16);

	auto spf = streaming::GetStreamingPackfileByIndex(collectionIndex);
	auto collection = (rage::fiCollection*)(spf ? spf->packfile : nullptr);

	if (!collection && collectionIndex == 0)
	{
		collection = getRawStreamer();
	}

	std::string name = fmt::sprintf("unknown - handle %08x", handle);
	std::string metaData;

	// get the input bytes
	auto compBytes = fmt::sprintf("%02x %02x %02x %02x %02x %02x %02x %02x", nextIn[0], nextIn[1], nextIn[2], nextIn[3], nextIn[4], nextIn[5], nextIn[6], nextIn[7]);

	// get collection metadata
	if (collection)
	{
		name = collection->GetEntryName(fileIndex);

		if (collectionIndex == 0)
		{
			// get the _raw_ file name
			char fileNameBuffer[1024];
			strcpy(fileNameBuffer, "CfxRequest");

			collection->GetEntryNameToBuffer(fileIndex, fileNameBuffer, sizeof(fileNameBuffer));

			auto virtualDevice = vfs::GetDevice(fileNameBuffer);

			// call into RCD
			GetRcdDebugInfoExtension ext;
			ext.fileName = fileNameBuffer;

			virtualDevice->ExtensionCtl(VFS_GET_RCD_DEBUG_INFO, &ext, sizeof(ext));

			metaData = ext.outData;
		}
	}
	else
	{
		metaData = "Null fiCollection.";
	}

	FatalError("Failed to call inflate() for streaming file %s.\n\n"
		"Error: %d: %s\nRead bytes: %s\nTotal in: %d\nAvailable in: %d\n"
		"%s\n\nPlease try restarting the game, or, if this occurs across servers, verifying your game files.",
		name,
		zlibError, (msg) ? msg : "(null)", compBytes, totalIn, availIn,
		metaData);
}

static void CompTrace()
{
	static struct : jitasm::Frontend
	{
		void InternalMain() override
		{
			mov(rcx, rdi);
			mov(rdx, r14);
			mov(r8d, eax);
			mov(r9, rbx);

			mov(rax, (uintptr_t)ErrorInflateFailure);
			jmp(rax);
		}
	} errorBit;

	hook::call_rcx(hook::get_pattern("B9 48 93 55 15 E8", 5), errorBit.GetCode());
}

static void* (*g_origSMPACreate)(void* a1, void* a2, size_t size, void* a4, bool a5);

static void* SMPACreateStub(void* a1, void* a2, size_t size, void* a4, bool a5)
{
	if (size == 0xD00000)
	{
		// free original allocation
		rage::GetAllocator()->Free(a2);

		size = 0x1200000;
		a2 = rage::GetAllocator()->Allocate(size, 16, 0);
	}

	return g_origSMPACreate(a1, a2, size, a4, a5);
}

static void* GetNvapi(uint32_t hash)
{
	auto patternString = fmt::sprintf("74 27 B9 %02X %02X %02X %02x FF 15", hash & 0xFF, (hash >> 8) & 0xFF, (hash >> 16) & 0xFF, (hash >> 24) & 0xFF);
	auto p = hook::get_pattern(patternString, -0x97);

	return p;
}

static int NvAPI_Stereo_IsEnabled(bool* enabled)
{
	*enabled = 1;
	return 0;
}

static int NvAPI_Stereo_CreateHandleFromIUnknown(void* iunno, uintptr_t* hdl)
{
	*hdl = 1;
	return 0;
}

static int NvAPI_Stereo_Activate(uintptr_t hdl)
{
	return 0;
}

static int NvAPI_Stereo_IsActivated(uintptr_t hdl, uint8_t* on)
{
	*on = 1;
	return 0;
}

static void HookStereo()
{
	hook::jump(GetNvapi(0x348FF8E1), NvAPI_Stereo_IsEnabled);
	hook::jump(GetNvapi(0xAC7E37F4), NvAPI_Stereo_CreateHandleFromIUnknown);
	hook::jump(GetNvapi(0xF6A1AD68), NvAPI_Stereo_Activate);
	hook::jump(GetNvapi(0x1FB0BC30), NvAPI_Stereo_IsActivated);
}


static void (*g_origSceneLoaderScan)(char* loader, uint8_t flags1, uint8_t flags2, uint8_t flags3);

static void SceneLoaderScan(char* loader, uint8_t flags1, uint8_t flags2, uint8_t flags3)
{
	g_origSceneLoaderScan(loader, flags1, flags2, flags3);

	auto mds = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
	atPoolBase* pool = (atPoolBase*)((char*)mds + 56);

	atArray<int>& indices = *(atArray<int>*)(loader + 80);
	for (int i = 0; i < indices.GetCount(); i++)
	{
		if (!pool->GetAt<void>(indices[i]))
		{
			// move the last item to the current position
			--indices.m_count;
			indices[i] = indices[indices.m_count];

			// iterate this one again
			--i;
		}
	}
}

static HookFunction hookFunction([]()
{
	// crash fix: popgroup unloading does an unknown streaming flush, which we don't want
	hook::put<uint8_t>(hook::get_pattern("33 D2 E8 ? ? ? ? B2 01 8A CA E8 ? ? ? ? 48", 27), 0xC3);

	// crash fix: sceneloader doesn't check if mapdatas obtained from boxstreamer still exist
	// we'll check for that, as removing anything from boxstreamer is weird
	MH_Initialize();
	MH_CreateHook(hook::get_call(hook::get_pattern("0F 29 81 80 00 00 00 F3 0F 11 99 90 00 00 00 E9", 15)), SceneLoaderScan, (void**)&g_origSceneLoaderScan);
	MH_EnableHook(MH_ALL_HOOKS);

#if 0
	hook::jump(hook::pattern("48 8B 48 08 48 85 C9 74  0C 8B 81").count(1).get(0).get<char>(-0x10), ReturnTrue);
	hook::put<uint8_t>(hook::pattern("80 3D ? ? ? ? ? 48 8B F1 74 07 E8 ? ? ? ? EB 05").count(1).get(0).get<void>(0xA), 0xEB);
	hook::put<uint8_t>(hook::pattern("0F 8E ? ? 00 00 80 3D ? ? ? ? 00 74 07 E8").count(1).get(0).get<void>(0xD), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 12 48 FF C3 48 83 C0 04 48 81 FB").count(1).get(0).get<void>(-0xB), 0xEB);
	hook::put<uint8_t>(hook::pattern("74 63 45 8D 47 02 E8").count(1).get(0).get<void>(0), 0xEB);

	/*{
		auto m = hook::pattern("44 89 7C 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}

	{
		auto m = hook::pattern("44 89 44 24 60 81 FA DB  3E 14 7D 74 16").count(1).get(0);

		hook::put<uint8_t>(m.get<void>(5), 0x90);
		hook::put<uint8_t>(m.get<void>(6), 0xBA);
		hook::put<uint32_t>(m.get<void>(7), HashString("meow"));
		hook::nop(m.get<void>(11), 2);
	}*/

	hook::nop(hook::pattern("0F B6 05 ? ? ? ? 40 8A BB").count(1).get(0).get<char>(0), 0x7);
	hook::put<uint8_t>(hook::pattern("48 83 C6 04 49 2B EC 75 CB").count(1).get(0).get<void>(0x15), 0xEB);

	hook::put<uint8_t>(hook::pattern("F6 05 ? ? ? ? ? 74 08 84 C0 0F 84").count(1).get(0).get<void>(0x18), 0xEB);
#endif

	if (!CfxIsSinglePlayer())
	{
		// passenger stuff?
		hook::put<uint16_t>(hook::get_pattern("8B 45 30 48 8B 4F 20 41 BE FF FF 00 00", -6), 0xE990);

		// population zone selection for network games
		hook::put<uint8_t>(hook::pattern("74 63 45 8D 47 02 E8").count(1).get(0).get<void>(0), 0xEB);

		// scenario netgame checks (NotAvailableInMultiplayer)
		hook::put<uint8_t>(hook::get_pattern("74 0D 8B 83 84 00 00 00 C1 E8 0F A8 01", 0), 0xEB);
		hook::put<uint8_t>(hook::get_pattern("74 1A 8B 49 38 E8", 0), 0xEB);

		// population netgame check
		hook::put<uint16_t>(hook::get_pattern("0F 84 8F 00 00 00 8B 44 24 40 8B C8"), 0xE990);

		// additional netgame checks for scenarios

		// 1737<
		if (!xbr::IsGameBuildOrGreater<2060>())
		{
			hook::nop(hook::get_pattern("B2 04 75 65 80 7B 39", 2), 2);
		}
		else if (xbr::IsGameBuildOrGreater<2189>())
		{
			hook::nop(hook::get_pattern("41 B0 04 75 72 0F B7 43 3A", 3), 2);
		}
		else
		{
			hook::nop(hook::get_pattern("41 B9 FF 01 00 00 BA FF 00 00 00 75 6E", 11), 2);
		}

		hook::put<uint8_t>(hook::get_pattern("74 24 84 D2 74 20 8B 83", 4), 0xEB);
		hook::put<uint8_t>(hook::get_pattern("84 D2 75 41 8B 83", 0x5F), 0xEB);
		//hook::put<uint8_t>(hook::get_pattern("40 B6 01 74 52 F3 0F 10 01", 3), 0xEB); // this skips a world grid check, might be bad!

		// another scenario cluster network game check
		hook::put<uint8_t>(hook::get_pattern("80 78 1A 00 74 0F", 4), 0xEB);

		// more netgame (scenariovehicleinfo)
		hook::put<uint8_t>(hook::get_pattern("74 42 84 C0 75 42 83 C8"), 0xEB);

		// disabling animal types
		//hook::jump(hook::pattern("48 8B 48 08 48 85 C9 74  0C 8B 81").count(1).get(0).get<char>(-0x10), ReturnTrue);
		hook::jump(hook::get_pattern("75 1A 38 99 54 01 00 00 75 0E", -0xE), ReturnTrue);

		// scenario point network game check
		hook::put<uint8_t>(hook::get_pattern("74 0D 3C 02 0F 94 C0 38 05", 0), 0xEB);

		// netgame checks for vehicle fuel tank leaking
		hook::nop(hook::get_pattern("48 83 65 7F 00 0F 29 45 27 48 8D 45 77", -0xE3), 7);
		hook::nop(hook::get_call(hook::get_pattern<char>("40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 84 C0 74 32 8A 83 D8", 9)) + 0x19, 2);
	}

	// increase the heap size for allocator 0
	hook::put<uint32_t>(hook::get_pattern("83 C8 01 48 8D 0D ? ? ? ? 41 B1 01 45 33 C0", 17), 700 * 1024 * 1024); // 700 MiB, default in 323 was 412 MiB

	// 1737+: increase rline allocator size using a hook (as Arxan)
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("49 63 F0 48 8B EA B9 07 00 00 00", -0x29), SMPACreateStub, (void**)&g_origSMPACreate);
	MH_EnableHook(MH_ALL_HOOKS);

	// don't pass flag 4 for streaming requests of netobjs
	hook::put<int>(hook::get_pattern("BA 06 00 00 00 41 23 CE 44 33 C1 44 23 C6 41 33", 1), 2);

	// allow all procedural objects in network games
	hook::put<uint8_t>(hook::get_pattern("F6 42 30 20 75 09", 4), 0xEB);

	// make GTA default rage::fwMapData::ms_entityLevelCap to PRI_OPTIONAL_LOW, not PRI_OPTIONAL_MEDIUM (RAGE suite defaults)
	hook::put<uint32_t>(hook::get_pattern("BB 02 00 00 00 39 1D", 1), 3);

	// trace ERR_GEN_ZLIB_2 errors
	CompTrace();

	// don't disable low-priority objects when LOD distance is <20%
	hook::nop(hook::get_pattern("0F 2F 47 24 0F 93 05", 4), 7);

	// don't clamp on-foot FOV to 1.0
	if (!xbr::IsGameBuildOrGreater<2060>())
	{
		hook::nop(hook::get_pattern("0F 2F C6 77 03 0F 28 F0 48 8B 83 ? ? ? ? F3 44", 3), 2);
	}
	else
	{
		hook::nop(hook::get_call(hook::get_pattern<char>("48 8B 9F ? ? ? ? 48 8B CF E8 ? ? ? ? 48 8B D6", 10)) + 0x26, 2);
	}

	// misc fix: pause menu control delay on exit
	hook::put<uint32_t>(hook::get_pattern("BA ? ? ? ? 48 8B C8 E8 ? ? ? ? 8B 05", 1), 0);

	// fwBoxStreamerVariable: relocate internal multi-node BVH traversal list and make it bigger (1000 limit is not enough
	// if having lots of mapdata with 'full-map-size' extents loaded that will always pass)
	auto mnbvhList = hook::AllocateStubMemory(4096 * 8);
	
	{
		// GetIntersectingAABB
		auto location = hook::get_pattern<char>("0F 28 0A 48 8B 49 08 4C 8D 25", 10);
		hook::put<int32_t>(location, (char*)mnbvhList - location + 4);
		hook::put<int32_t>(location + 31, 4004);
	}

	{
		// GetIntersectingLine
		auto location = hook::get_pattern<char>("48 8B 49 08 4C 8D 3D", 7);
		hook::put<int32_t>(location, (char*)mnbvhList - location + 4);
		hook::put<int32_t>(location + 8, 4004);
	}
});
