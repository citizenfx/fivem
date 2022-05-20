#include <StdInc.h>
#include <Hooking.h>

#include "ICoreGameInit.h"
#include "gameSkeleton.h"

#include <nutsnbolts.h>

static void** g_CCameraSaveStructure_parser;
static void** g_parManager;

static hook::thiscall_stub<bool(void*, const char*, const char*, void* structure, void* data, int _0, uint64_t* _settings)> _SaveFromStructure([]
{
	return hook::get_pattern("48 85 C0 74 2A 48 8B 44 24 70", -0x27);
});

static hook::thiscall_stub<bool(void*, const char*, const char*, void* structure, void* data, int _0, uint64_t* _settings)> _CreateAndLoadAnyType([]
{
	return hook::get_call(hook::get_pattern("C6 44 24 28 01 48 89 44 24 20 E8 ? ? ? ? 83 0D", 10));
});


static hook::cdecl_stub<void(int ns, uint32_t hash, const char* string)> rage__atHashStringNamespaceSupport__AddString([]
{
	return hook::get_call(hook::get_pattern("B9 04 00 00 00 8B D0 89 07 E8", 9));
});

static void (*CCameraSaveStructure__PreSave)(void* struc);

static void AddStrings()
{
	static bool added = ([]
	{
		for (auto str : {
			 "ON_FOOT",
			 "IN_VEHICLE",
			 "ON_BIKE",
			 "IN_BOAT",
			 "IN_AIRCRAFT",
			 "IN_SUBMARINE",
			 "IN_HELI",
			 "IN_TURRET",
			 "THIRD_PERSON_NEAR",
			 "THIRD_PERSON_MEDIUM",
			 "THIRD_PERSON_FAR",
			 "CINEMATIC",
			 "FIRST_PERSON" })
		{
			rage__atHashStringNamespaceSupport__AddString(1, HashString(str), str);
		}

		return true;
	})();
}

static void LoadCameraSave()
{
	AddStrings();

	char data[24] = { 0 };
	data[0] = 1;

	char* struc = (char*)*g_CCameraSaveStructure_parser;

	_CreateAndLoadAnyType(*g_parManager, "fxd:/camera_save_structure.xml", "", *g_CCameraSaveStructure_parser, data, 0, nullptr);
}

static void StoreCameraSave()
{
	AddStrings();

	char data[24] = { 0 };
	data[0] = 1;

	char* struc = (char*)*g_CCameraSaveStructure_parser;

	_SaveFromStructure(*g_parManager, "fxd:/camera_save_structure.xml", "", *g_CCameraSaveStructure_parser, data, 0, nullptr);
}

static int** g_contextViewModeList;
static int g_lastContextViewModes[8];

static HookFunction _([]()
{
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 48 89 05 ? ? ? ? 48 85 C0 74 37 33 C9", 5);
		g_contextViewModeList = hook::get_address<int**>(location, 3, 7);
	}

	{
		auto location = hook::get_pattern("BA 13 00 00 00 48 8D 48 10 48 8B 01", 15);
		g_parManager = hook::get_address<void**>(location, 3, 7);
	}

	{
		auto location = hook::get_pattern<char>("89 48 10 C6 00 01 EB 03 48 8B C1 48 8B", 0x19);
		hook::set_call(&CCameraSaveStructure__PreSave, location - 8);
		g_CCameraSaveStructure_parser = hook::get_address<void**>(location, 3, 7);
	}

	// support atHashValue members as string member, too
	{
		auto vtable = hook::get_address<void**>(hook::get_pattern("48 8B 5D 0F 48 83 65 0F 00 48 8D 05", 9), 3, 7);
		hook::put(&vtable[40], vtable[39]);
	}

	// and as string key (using same namespace 1)
	{
		auto location = hook::get_pattern("74 31 FF C9 0F 84 B9 00 00 00 FF C9 0F 84", 4);
		hook::nop(location, 6);
		hook::put<uint16_t>(location, 0x2D74);
	}

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			static auto icgi = Instance<ICoreGameInit>::Get();

			if (!icgi->HasVariable("storyMode"))
			{
				LoadCameraSave();

				memcpy(g_lastContextViewModes, *g_contextViewModeList, sizeof(g_lastContextViewModes));
			}
		}
	}, INT32_MIN);

	OnMainGameFrame.Connect([]
	{
		static auto icgi = Instance<ICoreGameInit>::Get();

		if (icgi->HasVariable("storyMode") || !*g_contextViewModeList)
		{
			return;
		}

		static uint64_t dirtyTime;

		if (memcmp(*g_contextViewModeList, g_lastContextViewModes, sizeof(g_lastContextViewModes)) != 0)
		{
			dirtyTime = GetTickCount64();
			memcpy(g_lastContextViewModes, *g_contextViewModeList, sizeof(g_lastContextViewModes));
		}

		if (dirtyTime > 0)
		{
			if ((GetTickCount64() - dirtyTime) > 5000)
			{
				StoreCameraSave();
				dirtyTime = 0;
			}
		}
	});
});
