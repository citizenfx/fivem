/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <CoreConsole.h>
#include "ICoreGameInit.h"
#include "fiDevice.h"

#include <scrEngine.h>
#include <gameSkeleton.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>

#include <GameInit.h>

#include <skyr/url.hpp>

#include "Hooking.h"

static std::string g_overrideNextLoadedLevel;
static std::string g_nextLevelPath;

static bool g_wasLastLevelCustom;

static void(*g_origLoadLevelByIndex)(int);
static void(*g_loadLevel)(const char* levelPath);

enum NativeIdentifiers : uint64_t
{
	GET_PLAYER_PED = 0x275F255ED201B937,
	SET_ENTITY_COORDS = 0x06843DA7060A026B,
	SHUTDOWN_LOADING_SCREEN = 0xFC179D7E8886DADF,
	DO_SCREEN_FADE_IN = 0x6A053CF596F67DF7
};

class SpawnThread : public GtaThread
{
private:
	int m_doInitThingsIn;

public:
	SpawnThread()
	{
		m_doInitThingsIn = 4;
	}

	virtual rage::eThreadState Reset(uint32_t scriptHash, void* pArgs, uint32_t argCount) override
	{
		m_doInitThingsIn = 4;

		return GtaThread::Reset(scriptHash, pArgs, argCount);
	}

	virtual void DoRun() override
	{
		uint32_t playerPedId = NativeInvoke::Invoke<GET_PLAYER_PED, uint32_t>(0xFF);

		if (m_doInitThingsIn >= 0)
		{
			if (m_doInitThingsIn == 0)
			{
				NativeInvoke::Invoke<SHUTDOWN_LOADING_SCREEN, int>();
				NativeInvoke::Invoke<DO_SCREEN_FADE_IN, int>(0);

				NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, 35.0f, 35.0f, 102.0f);
			}

			m_doInitThingsIn--;
		}
	}
};

static bool isLoadLevel;

static void DoLoadLevel(int index)
{
	isLoadLevel = true;
	g_wasLastLevelCustom = false;

	if (g_overrideNextLoadedLevel.empty())
	{
		g_origLoadLevelByIndex(index);

		return;
	}

	// we're trying to override the level - try finding the level asked for.
	bool foundLevel = false;

	auto testLevel = [](const char* path)
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
	g_wasLastLevelCustom = (g_overrideNextLoadedLevel != "gta5" && g_overrideNextLoadedLevel.find("/gta5") == std::string::npos);

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

static HookFunction hookFunction([]()
{
	/*char* levelCaller = hook::pattern("0F 94 C2 C1 C1 10 33 CB 03 D3 89 0D").count(1).get(0).get<char>(46);
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
	}*/
});

static SpawnThread spawnThread;

static void LoadLevel(const char* levelName)
{
	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();

	g_overrideNextLoadedLevel = levelName;

	if (!gameInit->GetGameLoaded())
	{
		if (!gameInit->HasVariable("storyMode")/* && !gameInit->HasVariable("localMode")*/)
		{
			rage::scrEngine::CreateThread(&spawnThread);
		}

		gameInit->LoadGameFirstLaunch([]()
		{
			return true;
		});
	}
	else
	{
		//gameInit->KillNetwork((wchar_t*)1);

		gameInit->ReloadGame();
	}

	gameInit->ShAllowed = true;
}

class SPResourceMounter : public fx::ResourceMounter
{
public:
	SPResourceMounter(fx::ResourceManager* manager)
		: m_manager(manager)
	{

	}

	virtual bool HandlesScheme(const std::string& scheme) override
	{
		return (scheme == "file");
	}

	virtual pplx::task<fwRefContainer<fx::Resource>> LoadResource(const std::string& uri) override
	{
		auto uriParsed = skyr::make_url(uri);

		fwRefContainer<fx::Resource> resource;

		if (uriParsed)
		{
			auto pathRef = uriParsed->pathname();
			auto fragRef = uriParsed->hash().substr(1);

			if (!pathRef.empty() && !fragRef.empty())
			{
				std::vector<char> path;
				std::string pr = pathRef.substr(1);
				//network::uri::decode(pr.begin(), pr.end(), std::back_inserter(path));

				resource = m_manager->CreateResource(fragRef);
				resource->LoadFrom(pr);
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

static InitFunction initFunction([]()
{
	rage::fiDevice::OnInitialMount.Connect([]()
	{
		std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>, wchar_t> converter;
		std::string usermapsPath = converter.to_bytes(MakeRelativeCitPath(L"usermaps/"));
		rage::fiDeviceRelative* device = new rage::fiDeviceRelative();
		device->SetPath(usermapsPath.c_str(), true);
		device->Mount("usermaps:/");
	});

	static ConsoleCommand loadLevelCommand("loadlevel", [](const std::string& level)
	{
		LoadLevel(level.c_str());
	});

	static ConsoleCommand storyModeyCommand("storymode", []()
	{
		Instance<ICoreGameInit>::Get()->SetVariable("storyMode");
		LoadLevel("rdr3");
	});

	static ConsoleCommand localGameCommand("localGame", [](const std::string& resourceDir)
	{
		//Instance<ICoreGameInit>::Get()->SetVariable("localMode");

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		resourceManager->AddMounter(new SPResourceMounter(resourceManager));

		auto resourceRoot = "usermaps:/resources/" + resourceDir;

		skyr::url_record record;
		record.scheme = "file";

		skyr::url url{ std::move(record) };
		url.set_pathname(resourceRoot);
		url.set_hash(resourceDir);

		resourceManager->AddResource(url.href())
			.then([](fwRefContainer<fx::Resource> resource)
		{
			trace("start?\n");
			resource->Start();
		});

		static ConsoleCommand localRestartCommand("localRestart", [resourceRoot, resourceDir, resourceManager]()
		{
			auto res = resourceManager->GetResource(resourceDir);
			res->GetComponent<fx::ResourceMetaDataComponent>()->LoadMetaData(resourceRoot);

			res->Stop();
			res->Start();
		});

		LoadLevel("rdr3");
	});

	static ConsoleCommand loadLevelCommand2("invoke-levelload", [](const std::string& level)
	{
		LoadLevel(level.c_str());
	});

	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::InitFunctionType::INIT_SESSION && isLoadLevel)
		{
			Instance<ICoreGameInit>::Get()->SetVariable("networkInited");
		}
	});

	OnKillNetworkDone.Connect([]()
	{
		isLoadLevel = false;
	});

	/*static ConsoleCommand mehCommand("tryLoad", []()
	{
		((void(*)(const uint32_t&, const uint32_t&))0x14106B6B4)(HashString("landing_page"), HashString("to_sp"));
	});*/
});
