#include <StdInc.h>
#include <Hooking.h>

#include <MinHook.h>

#include <shlwapi.h>
#include <shlobj.h>

#include <fiDevice.h>

#pragma comment(lib, "shlwapi.lib")

static void AppendPathComponent(std::wstring& value, const wchar_t* component)
{
	value += component;

	if (GetFileAttributes(value.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		CreateDirectoryW(value.c_str(), nullptr);
	}
}

// get our Citizen save game directory
static std::wstring GetCitizenSavePath()
{
	PWSTR saveBasePath;

	// get the 'Saved Games' shell directory
	if (SUCCEEDED(SHGetKnownFolderPath(FOLDERID_SavedGames, 0, nullptr, &saveBasePath)))
	{
		// create a STL string and free the used memory
		std::wstring savePath(saveBasePath);

		CoTaskMemFree(saveBasePath);

		// append our path components
		AppendPathComponent(savePath, L"\\VMP");
		AppendPathComponent(savePath, L"\\GTA5");

		// append a final separator
		savePath += L"\\";

		// and return the path
		return savePath;
	}

	return MakeRelativeCitPath(L"saves");
}

static bool(*g_origGetProfileFileName)(int userIdx, char outFileName[256], const char* suffix);

static bool GetProfileFileName(int userIdx, char outFileName[256], const char* suffix)
{
	auto saveRoot = ToNarrow(GetCitizenSavePath());

	if (suffix)
	{
		saveRoot += suffix;
	}

	strncpy(outFileName, saveRoot.c_str(), 256);

	return true;
}

static int ReturnTrue()
{
	return 1;
}

struct ProfileSettingsStruct
{
	// +0
	char pad[32];

	// +32
	bool loaded;
	bool pendingLoad;

	// +34
	bool pad2[2];

	// +36
	char pad3[4];

	// +40
	uint64_t gamerId;

	// ...
};

static uint8_t dummyScProfile[512];
static ProfileSettingsStruct** g_profileSettings;

static bool (*g_origIsScSignedIn)();

static void* GetDummyScProfile()
{
	if (!dummyScProfile[0])
	{
		memset(dummyScProfile, 0xCC, sizeof(dummyScProfile));
	}

	if (auto profileSettings = *g_profileSettings)
	{
		static bool wasLoaded = false;

		// if we ever were loaded, don't try to bump the gamer ID
		if (profileSettings->loaded)
		{
			wasLoaded = true;
		}

		// bump the gamer ID if we were signed in
		if (!wasLoaded && !profileSettings->pendingLoad)
		{
			if (g_origIsScSignedIn())
			{
				memset(dummyScProfile, 0xCD, sizeof(dummyScProfile));
			}
		}
	}

	// 'update' gamerId flags to be valid for newer game versions
	for (auto offset : { 16, 200 })
	{
		dummyScProfile[offset + 8] = 3; // online?
		dummyScProfile[offset + 9] = 0; // index
	}

	return dummyScProfile;
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("FF 51 10 85 C0 78 32", -0x74), GetProfileFileName, (void**)&g_origGetProfileFileName);

	// force parts of game to think SC is always signed on
	MH_CreateHook(hook::get_pattern("FF 52 08 84 C0 74 04 B0 01 EB", -0x26), ReturnTrue, (void**)&g_origIsScSignedIn);
	MH_EnableHook(MH_ALL_HOOKS);

	// load user profile settings with a 'fake' profile all the time
	hook::call(hook::get_pattern("40 8A F2 48 8B D9 E8 ? ? ? ? 45 33 F6 48", 6), GetDummyScProfile);

	g_profileSettings = hook::get_address<ProfileSettingsStruct**>(hook::get_pattern("48 8B 0D ? ? ? ? 45 33 C0 B2 01 48 8B"), 3, 7);

	// add save path to user:/ device stack
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->SetPath(ToNarrow(GetCitizenSavePath()).c_str(), true);
		device->Mount("fxu:/");
	});
});
