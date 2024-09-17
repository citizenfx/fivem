/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <CrossBuildRuntime.h>
#include <CoreConsole.h>
#include "ICoreGameInit.h"
#include "fiDevice.h"

#include <scrEngine.h>

#include <ResourceManager.h>
#include <ResourceMetaDataComponent.h>

#include <VFSManager.h>
#include <boost/algorithm/string.hpp>

#include <concurrent_queue.h>

#include "atArray.h"

void DLL_IMPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);

namespace streaming
{
void AddDataFileToLoadList(const std::string& type, const std::string& path);
}

#include <skyr/url.hpp>

#include "Hooking.h"
#include "Hooking.Stubs.h"

static std::string g_overrideNextLoadedLevel;
static std::string g_nextLevelPath;

static bool g_wasLastLevelCustom;
static bool g_gameUnloaded = false;

static void(*g_origLoadLevelByIndex)(int);
static void(*g_loadLevel)(const char* levelPath);

enum NativeIdentifiers : uint64_t
{
	PLAYER_PED_ID = 0xD80958FC74E988A6,
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
		: m_doInityThings(false)
	{
	}

	void ResetInityThings()
	{
		m_doInityThings = true;
	}

	virtual void DoRun() override
	{
		if (m_doInityThings)
		{
			uint32_t playerPedId = NativeInvoke::Invoke<PLAYER_PED_ID, uint32_t>();

			NativeInvoke::Invoke<SHUTDOWN_LOADING_SCREEN, int>();
			NativeInvoke::Invoke<DO_SCREEN_FADE_IN, int>(0);

			NativeInvoke::Invoke<SET_ENTITY_COORDS, int>(playerPedId, 293.089f, 180.466f, 104.301f);
			NativeInvoke::Invoke<0x428CA6DBD1094446, int>(NativeInvoke::Invoke<0xD80958FC74E988A6, int>(), false);

			if (Instance<ICoreGameInit>::Get()->HasVariable("editorMode"))
			{
				NativeInvoke::Invoke<0x49DA8145672B2725, int>();
				Instance<ICoreGameInit>::Get()->ClearVariable("editorMode");
			}

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

	return (!g_wasLastLevelCustom);
}

struct CDataFileMgr__ContentChangeSet
{
	// technically, some atString?
	atArray<char> changeSetName;
	// ...
};

static void (*g_orig_CFileLoader__BuildContentChangeSetActionList)(const CDataFileMgr__ContentChangeSet& changeset, void* outArray, int a3, int a4, int a5);

static void CFileLoader__BuildContentChangeSetActionList_Hook(const CDataFileMgr__ContentChangeSet& changeset, void* outArray, int a3, int a4, int a5)
{
	if (g_wasLastLevelCustom)
	{
		// CCS_PATCHDAY27_NG_STREAMING_MAP lacks a level condition and breaks custom levels
		const char* str = (changeset.changeSetName.m_size > 0) ? changeset.changeSetName.m_offset : "";

		if (HashString(str) == HashString("CCS_PATCHDAY27_NG_STREAMING_MAP"))
		{
			return;
		}
	}

	return g_orig_CFileLoader__BuildContentChangeSetActionList(changeset, outArray, a3, a4, a5);
}

static HookFunction hookFunction([] ()
{
	char* levelCaller = xbr::IsGameBuildOrGreater<2060>() ? hook::pattern("33 D0 81 E2 FF 00 FF 00 33 D1 48").count(1).get(0).get<char>(0x33) : hook::pattern("0F 94 C2 C1 C1 10 33 CB 03 D3 89 0D").count(1).get(0).get<char>(46);
	char* levelByIndex = hook::get_call(levelCaller);

	hook::set_call(&g_origLoadLevelByIndex, levelCaller);
	hook::call(levelCaller, DoLoadLevel);

	hook::set_call(&g_loadLevel, levelByIndex + 0x1F);

	g_orig_CFileLoader__BuildContentChangeSetActionList = hook::trampoline(hook::get_pattern("E8 ? ? ? ? 44 8A A4 24 80 00 00 00 33 ED 8B F5 66", -0x30), CFileLoader__BuildContentChangeSetActionList_Hook);

	// change set applicability
	hook::jump(hook::pattern("40 8A EA 48 8B F9 B0 01 76 43 E8").count(1).get(0).get<void>(-0x19), IsLevelApplicable);

	// change set condition evaluator's $level variable comparer
	{
		char* location = hook::pattern("EB 03 4C 8B F3 48 8D 05 ? ? ? ? 48 8B CE 49").count(1).get(0).get<char>(8);

		hook::jump(location + *(int32_t*)location + 4, DoesLevelHashMatch);
	}
});

enum class Mode : uint8_t
{
	LOCAL_MODE,
	STORY_MODE,
	EDITOR_MODE,
	LEVEL_LOAD,
};

static SpawnThread spawnThread;

static concurrency::concurrent_queue<std::function<void()>> g_onShutdownQueue;

/// Does the game mode require resetting the SpawnThread.
static inline bool DoesModeRequiresReset(Mode mode)
{
	return mode == Mode::LEVEL_LOAD || mode == Mode::EDITOR_MODE;
}

/// Initialize correct ICoreGameInit variables for the given level mode.
static void SetCoreGameMode(Mode mode)
{
	auto setVariable = [](const std::string& variable, bool enabled)
	{
		enabled ? Instance<ICoreGameInit>::Get()->SetVariable(variable)
				: Instance<ICoreGameInit>::Get()->ClearVariable(variable);
	};

	// editorMode implies localMode; editorMode is cleared after _ACTIVATE_ROCKSTAR_EDITOR.
	setVariable("storyMode", mode == Mode::STORY_MODE);
	setVariable("localMode", mode == Mode::LOCAL_MODE || mode == Mode::EDITOR_MODE);
	setVariable("editorMode", mode == Mode::EDITOR_MODE);
}

static void LoadLevel(const char* levelName, Mode mode)
{
	static auto gameInit = Instance<ICoreGameInit>::Get();

	gameInit->SetVariable("networkInited");

	g_overrideNextLoadedLevel = levelName;

	if (!gameInit->GetGameLoaded())
	{
		SetCoreGameMode(mode);
		if (DoesModeRequiresReset(mode))
		{
			spawnThread.ResetInityThings();
		}

		gameInit->LoadGameFirstLaunch([] ()
		{
			return true;
		});

		gameInit->ShAllowed = true;
	}
	else
	{
		gameInit->KillNetwork((wchar_t*)1);

		auto fEvent = ([mode]()
		{
			gameInit->ReloadGame();

			SetCoreGameMode(mode);
			if (DoesModeRequiresReset(mode))
			{
				spawnThread.ResetInityThings();
			}

			gameInit->SetVariable("networkInited");
			gameInit->ShAllowed = true;
		});

		if (g_gameUnloaded)
		{
			fEvent();

			g_gameUnloaded = false;
		}
		else
		{
			g_onShutdownQueue.push(fEvent); // OnShutdownSession
		}
	}
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

				resource = m_manager->CreateResource(fragRef, this);
				resource->LoadFrom(pr);
			}
		}

		return pplx::task_from_result<fwRefContainer<fx::Resource>>(resource);
	}

private:
	fx::ResourceManager* m_manager;
};

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	rage::ResourceFlags flags; // out
};

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

	static ConsoleCommand loadLevelCommand("loadlevel", [](const std::string& level)
	{
		LoadLevel(level.c_str(), Mode::LEVEL_LOAD);
	});

	static ConsoleCommand storyModeyCommand("storymode", []()
	{
		LoadLevel("gta5", Mode::STORY_MODE);
	});

	static ConsoleCommand editorModeCommand("replayEditor", []()
	{
		LoadLevel("gta5", Mode::EDITOR_MODE);
	});

	static ConsoleCommand localGameCommand("localGame", [](const std::string& resourceDir)
	{
		Instance<ICoreGameInit>::Get()->SetData("localResource", resourceDir);

		fx::ResourceManager* resourceManager = Instance<fx::ResourceManager>::Get();
		resourceManager->AddMounter(new SPResourceMounter(resourceManager));

		auto resourceRoot = "usermaps:/resources/" + resourceDir;

		skyr::url_record record;
		record.scheme = "file";

		skyr::url url{ std::move(record) };
		url.set_pathname(resourceRoot);
		url.set_hash(resourceDir);

		resourceManager->AddResource(url.href())
			.then([resourceDir, resourceRoot](fwRefContainer<fx::Resource> resource)
		{
			resource->Start();
		});

		// also tag with streaming files, wahoo!
		// #TODO: make recursive!
		// #TODO: maybe share this code once it does support recursion?
		vfs::FindData findData;
		auto mount = resourceRoot + "/stream/";
		auto device = vfs::GetDevice(mount);

		if (device.GetRef())
		{
			auto findHandle = device->FindFirst(mount, &findData);

			if (findHandle != INVALID_DEVICE_HANDLE)
			{
				bool shouldUseCache = false;
				bool shouldUseMapStore = false;

				do
				{
					if (!(findData.attributes & FILE_ATTRIBUTE_DIRECTORY))
					{
						std::string tfn = mount + findData.name;

						GetRagePageFlagsExtension data;
						data.fileName = tfn.c_str();
						device->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &data, sizeof(data));

						CfxCollection_AddStreamingFileByTag(resourceDir, tfn, data.flags);

						if (boost::algorithm::ends_with(tfn, ".ymf"))
						{
							shouldUseCache = true;
						}

						if (boost::algorithm::ends_with(tfn, ".ybn") || boost::algorithm::ends_with(tfn, ".ymap"))
						{
							shouldUseMapStore = true;
						}
					}
				} while (device->FindNext(findHandle, &findData));

				device->FindClose(findHandle);

				// in case of .#mf file
				if (shouldUseCache)
				{
					streaming::AddDataFileToLoadList("CFX_PSEUDO_CACHE", resourceDir);
				}

				// in case of .#bn/.#map file
				if (shouldUseMapStore)
				{
					streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
				}
			}
		}

		static ConsoleCommand localRestartCommand("localRestart", [resourceRoot, resourceDir, resourceManager]()
		{
			auto res = resourceManager->GetResource(resourceDir);
			res->GetComponent<fx::ResourceMetaDataComponent>()->LoadMetaData(resourceRoot);

			res->Stop();
			res->Start();
		});

		LoadLevel("gta5", Mode::LOCAL_MODE);
	});

	static ConsoleCommand loadLevelCommand2("invoke-levelload", [](const std::string& level)
	{
		LoadLevel(level.c_str(), Mode::LEVEL_LOAD);
	});

	rage::scrEngine::OnScriptInit.Connect([] ()
	{
		rage::scrEngine::CreateThread(&spawnThread);
	}, INT32_MAX);

	Instance<ICoreGameInit>::Get()->OnGameRequestLoad.Connect([]()
	{
		g_gameUnloaded = false;
	});

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		std::function<void()> fn;

		while (g_onShutdownQueue.try_pop(fn))
		{
			fn();
		}

		g_gameUnloaded = true;
	}, INT32_MAX);
});
