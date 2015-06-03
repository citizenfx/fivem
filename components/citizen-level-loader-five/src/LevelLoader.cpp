/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include "ConsoleHost.h"
#include "ICoreGameInit.h"
#include "fiDevice.h"

#include "Hooking.h"

static std::string g_overrideNextLoadedLevel;
static std::string g_nextLevelPath;

static void(*g_origLoadLevelByIndex)(int);
static void(*g_loadLevel)(const char* levelPath);

static void DoLoadLevel(int index)
{
	if (g_overrideNextLoadedLevel.empty())
	{
		g_origLoadLevelByIndex(index);

		return;
	}

	// we're trying to override the level - try finding the level asked for.
	bool foundLevel = false;

	auto testLevel = [] (const char* path)
	{
		std::string metaFile = std::string(path) + ".meta";

		rage::fiDevice* device = rage::fiDevice::GetDevice(metaFile.c_str(), true);

		if (device)
		{
			return (device->GetFileAttributes(metaFile.c_str()) != INVALID_FILE_ATTRIBUTES);
		}

		return false;
	};

	const char* levelPath = nullptr;
	
	// try usermaps
	levelPath = va("usermaps:/%s/%s", g_overrideNextLoadedLevel.c_str(), g_overrideNextLoadedLevel.c_str());
	foundLevel = testLevel(levelPath);

	if (!foundLevel)
	{
		levelPath = va("common:/data/levels/%s/%s", g_overrideNextLoadedLevel.c_str(), g_overrideNextLoadedLevel.c_str());
		foundLevel = testLevel(levelPath);

		if (!foundLevel)
		{
			std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
			std::wstring wideLevel = converter.from_bytes(g_overrideNextLoadedLevel);

			g_overrideNextLoadedLevel.clear();

			Instance<ICoreGameInit>::Get()->KillNetwork(va(L"Could not find requested level (%s) - loaded the default level instead.", wideLevel.c_str()));

			g_origLoadLevelByIndex(index);

			return;
		}
	}

	// clear the 'next' level
	g_overrideNextLoadedLevel.clear();
	
	// save globally to prevent va() reuse messing up
	g_nextLevelPath = levelPath;

	// load the level
	g_loadLevel(g_nextLevelPath.c_str());
}

static HookFunction hookFunction([] ()
{
	char* levelCaller = hook::pattern("0F 94 C2 C1 C1 10 33 CB 03 D3 89 0D").count(1).get(0).get<char>(46);
	char* levelByIndex = hook::get_call(levelCaller);

	hook::set_call(&g_origLoadLevelByIndex, levelCaller);
	hook::call(levelCaller, DoLoadLevel);

	hook::set_call(&g_loadLevel, levelByIndex + 0x1F);
});

static void LoadLevel(const char* levelName)
{
	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

	g_overrideNextLoadedLevel = levelName;

	if (!gameInit->GetGameLoaded())
	{
		gameInit->LoadGameFirstLaunch([] ()
		{
			return true;
		});
	}
	else
	{
		gameInit->KillNetwork((wchar_t*)1);

		gameInit->ReloadGame();
	}
}

static InitFunction initFunction([] ()
{
	rage::fiDevice::OnInitialMount.Connect([] ()
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string usermapsPath = converter.to_bytes(MakeRelativeCitPath(L"usermaps/"));

		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->SetPath(usermapsPath.c_str(), true);
		device->Mount("usermaps:/");
	});

	ConHost::OnInvokeNative.Connect([] (const char* type, const char* argument)
	{
		if (strcmp(type, "loadLevel") == 0)
		{
			LoadLevel(argument);
		}
	});
});