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

#include <scrEngine.h>

#include "Hooking.h"

static std::string g_overrideNextLoadedLevel;
static std::string g_nextLevelPath;

static bool g_wasLastLevelCustom;

static void(*g_origLoadLevelByIndex)(int);
static void(*g_loadLevel)(const char* levelPath);

enum NativeIdentifiers : uint64_t
{
	GET_PLAYER_PED = 0x43A66C31C68491C0,
	SET_ENTITY_COORDS = 0x621873ECE1178967,
	LOAD_SCENE = 0x4448EB75B4904BDB,
	SHUTDOWN_LOADING_SCREEN = 0x078EBE9809CCD637,
	DO_SCREEN_FADE_IN = 0xD4E8E24955024033
};

class SpawnThread : public GtaThread
{
private:
	bool m_doInityThings;

public:
	SpawnThread()
	{
		m_doInityThings = true;
	}

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		m_doInityThings = true;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
	}

	virtual void DoRun() override
	{
		uint32_t playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, uint32_t>(-1);

		if (m_doInityThings)
		{
			NativeInvoke::Invoke<LOAD_SCENE, int>(-426.858f, -957.54f, 3.621f);

			NativeInvoke::Invoke<SHUTDOWN_LOADING_SCREEN, int>();
			NativeInvoke::Invoke<DO_SCREEN_FADE_IN, int>(0);

			NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, -426.858f, -957.54f, 3.621f);

			m_doInityThings = false;
		}
	}
};

static void DoLoadLevel(int index)
{
	g_wasLastLevelCustom = false;

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
	
	// try hardcoded level name
	if (g_overrideNextLoadedLevel.find(':') != std::string::npos)
	{
		levelPath = va("%s", g_overrideNextLoadedLevel.c_str());
		foundLevel = testLevel(levelPath);
	}

	if (!foundLevel)
	{
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
	}

	// mark the level as being custom
	g_wasLastLevelCustom = true;

	// clear the 'next' level
	g_overrideNextLoadedLevel.clear();
	
	// save globally to prevent va() reuse messing up
	g_nextLevelPath = levelPath;

	// load the level
	g_loadLevel(g_nextLevelPath.c_str());
}

namespace streaming
{
	void DLL_EXPORT SetNextLevelPath(const std::string& path)
	{
		g_overrideNextLoadedLevel = path;
	}
}

static bool IsLevelApplicable()
{
	return (!g_wasLastLevelCustom);
}

static bool DoesLevelHashMatch(void* evaluator, uint32_t* hash)
{
	// technically we should verify the hash, as with the above - but as nobody writes DLCs assuming custom levels
	// we shouldn't care about this at all - non-custom is always MO_JIM_L11 (display label for 'gta5'), custom is never MO_JIM_L11

	trace("level hash match - was custom: %d\n", g_wasLastLevelCustom);

	return (!g_wasLastLevelCustom);
}

static HookFunction hookFunction([] ()
{
	char* levelCaller = hook::pattern("0F 94 C2 C1 C1 10 33 CB 03 D3 89 0D").count(1).get(0).get<char>(46);
	char* levelByIndex = hook::get_call(levelCaller);

	hook::set_call(&g_origLoadLevelByIndex, levelCaller);
	hook::call(levelCaller, DoLoadLevel);

	hook::set_call(&g_loadLevel, levelByIndex + 0x1F);

	// change set applicability
	hook::jump(hook::pattern("40 8A EA 48 8B F9 B0 01 76 43 E8").count(1).get(0).get<void>(-0x19), IsLevelApplicable);

	// change set condition evaluator's $level variable comparer
	{
		char* location = hook::pattern("EB 03 4C 8B F3 48 8D 05 ? ? ? ? 48 8B CE 49").count(1).get(0).get<char>(8);

		hook::jump(location + *(int32_t*)location + 4, DoesLevelHashMatch);
	}
});

static SpawnThread spawnThread;

static void LoadLevel(const char* levelName)
{
	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

	gameInit->SetVariable("networkInited");

	g_overrideNextLoadedLevel = levelName;

	if (!gameInit->GetGameLoaded())
	{
		rage::scrEngine::CreateThread(&spawnThread);

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

	gameInit->ShAllowed = true;
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