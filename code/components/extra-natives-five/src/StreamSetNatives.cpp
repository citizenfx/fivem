/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#ifdef GTA_FIVE

#include <ResourceManager.h>
#include <ResourceEventComponent.h>
#include <EventReassemblyComponent.h>
#include "fxScripting.h"
#include "Resource.h"
#include <HookFunction.h>
#include <Hooking.Patterns.h>
#include <Hooking.Stubs.h>
#include <nutsnbolts.h>
#include <ScriptEngine.h>
#include <CachedResourceMounter.h>
#include <CachedResourceMounterWrap.h>
#include <HttpClient.h>
#include <fiDevice.h>
#include <VFSRagePackfile.h>
#include <VFSManager.h>
#include <NetLibrary.h>
#include <ResourceMetaDataComponent.h>
#include <NetLibraryResourcesComponent.h>
#include <rageVectors.h>
#include <scrEngine.h>
#include <Streaming.h>
#include <ICoreGameInit.h>

#define VFS_GET_RAGE_PAGE_FLAGS 0x20001

struct GetRagePageFlagsExtension
{
	const char* fileName; // in
	int version;
	rage::ResourceFlags flags; // out
};

struct StreamSetEntry
{
	std::string name;
	bool useMapStore;
};

DLL_IMPORT void CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags);
DLL_IMPORT void CfxCollection_RemoveStreamingTag(const std::string& tag);

DLL_IMPORT void LoadManifest(const char* tagName);
DLL_IMPORT void CleanupStreaming(std::set<std::string> tags = {});

DLL_IMPORT fwEvent<NetLibrary*> OnNetBindingsAttachToObject;
DLL_IMPORT fwEvent<> OnReloadMapStore;

DLL_IMPORT bool g_unloadingCfx;
DLL_IMPORT bool g_lockReload;

namespace streaming
{
	DLL_IMPORT void AddDataFileToLoadList(const std::string& type, const std::string& path);
};


static std::unordered_map<std::string, std::unordered_set<fx::Resource*>> s_StreamSetResources;
static std::unordered_map<std::string, StreamSetEntry> s_ActiveStreamSets;

static NetLibrary* s_netLibrary = nullptr;
static void* s_strStreamingLoaderMgr = nullptr;
static void* s_strStreamingInfomgr = nullptr;

static void(__fastcall* FlushStreamingLoader)(void* pThis) = nullptr;
static void(__fastcall* PurgeInfoRequestList)(void* pThis, uint16_t flags, bool a3) = nullptr;
static void(__fastcall* FlushInfoLoadedList)(void* pThis, uint16_t flags) = nullptr;


static void AfterStreamSet(const std::string& name, bool enable)
{

	if (!enable)
	{
		g_unloadingCfx = true;
		g_lockReload = true;

		auto tag = "streamset_" + name;
		CfxCollection_RemoveStreamingTag(tag);
		CleanupStreaming({ tag });

	}

	PurgeInfoRequestList(s_strStreamingInfomgr, 249, true);
	FlushStreamingLoader(s_strStreamingLoaderMgr);
	FlushInfoLoadedList(s_strStreamingInfomgr, 249);

	if (!enable)
	{
		g_unloadingCfx = false;
		g_lockReload = false;
	}
}

static void LoadStreamSetInternal(std::string setName)
{
	static auto resourceManager = Instance<fx::ResourceManager>::Get();
	static auto eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();
	static auto str = streaming::Manager::GetInstance();

	auto it = s_ActiveStreamSets.find(setName);

	if (it != s_ActiveStreamSets.end())
	{
		return;
	}

	s_ActiveStreamSets[setName] = { setName, false };

	trace("load stream set -> %s\n", setName);

	const std::string tag = "streamset_" + setName;

	std::unordered_set<fx::Resource*> caches;
	bool shouldUseMapStore = false;

	auto it2 = s_StreamSetResources.find(setName);

	if (it2 != s_StreamSetResources.end())
	{
		for (const auto& resource : it2->second)
		{
			std::string overlayFile = fmt::sprintf("%s.rpf", setName);
			std::string overlayPath = fmt::sprintf("cache:/%s/%s", resource->GetName(), overlayFile);

			auto mounter = resource->GetComponent<fx::CachedResourceMounterWrap>();
			auto entryList = resource->GetComponent<ResourceCacheEntryList>();

			// verify if we even have an entry for such
			if (entryList->GetEntry(overlayFile))
			{
				fwRefContainer<vfs::RagePackfile> packfile = new vfs::RagePackfile();
				std::string errorState;

				if (packfile->OpenArchive(overlayPath, &errorState))
				{
					auto device = vfs::GetDevice(fmt::sprintf("overlay:/%s/%s", setName, resource->GetName()));

					packfile->ForEachEntry([&](const std::string& name, bool isDirectory)
					{
						auto path = std::filesystem::path(name);
						auto fileName = path.filename().string();
						auto stem = path.stem().string();
						auto ext = path.extension().string();

						if (!ext.empty() && ext[0] == '.')
						{
							ext.erase(0, 1);
						}

						if (!isDirectory)
						{
							std::string tfn = fmt::sprintf("overlay:/%s/%s/%s", setName, resource->GetName(), name);
							auto p = std::filesystem::path(tfn);
							auto stem = p.stem().string();

							uint32_t hash = HashString(stem);

							trace("register [%s.%s:0x%x] -> %s", stem.c_str(), ext.c_str(), hash, tfn.c_str());

							GetRagePageFlagsExtension data;
							data.fileName = tfn.c_str();
							if (!device->ExtensionCtl(VFS_GET_RAGE_PAGE_FLAGS, &data, sizeof(data)))
							{
								trace(" [error]\n");
								return;
							}
							else
							{
								trace("\n");
							}

							CfxCollection_AddStreamingFileByTag(tag, tfn, data.flags);

							if (boost::algorithm::ends_with(fileName, ".ymf"))
							{
								caches.emplace(resource);
							}

							if (boost::algorithm::ends_with(fileName, ".ybn") || boost::algorithm::ends_with(fileName, ".ymap"))
							{
								shouldUseMapStore = true;
							}
						}
					});
				}
				else
				{
					trace("openArchive error\n");
				}
			}
		}
	}

	for (const auto& resource : caches)
	{
		streaming::AddDataFileToLoadList("CFX_PSEUDO_CACHE", resource->GetName());
	}

	s_ActiveStreamSets[setName].useMapStore = shouldUseMapStore;

	LoadManifest(tag.c_str());

	auto& evt = shouldUseMapStore ? OnReloadMapStore : OnMainGameFrame;

	evt.ConnectOnce([setName]()
	{
		AfterStreamSet(setName, true);
		eventManager->TriggerEvent2("onStreamSetLoad", {}, setName);
	});

	if (shouldUseMapStore)
	{
		streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
	}
}

static void ReleaseStreamSetInternal(std::string setName)
{
	static auto resourceManager = Instance<fx::ResourceManager>::Get();
	static auto eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();

	auto it = s_ActiveStreamSets.find(setName);

	if (it == s_ActiveStreamSets.end())
	{
		return;
	}

	trace("release stream set -> %s\n", setName);

	const std::string tag = "streamset_" + setName;

	s_ActiveStreamSets.erase(it);

	bool shouldUseMapStore = it->second.useMapStore;

	auto& evt = shouldUseMapStore ? OnReloadMapStore : OnMainGameFrame;

	evt.ConnectOnce([setName]()
	{
		AfterStreamSet(setName, false);
		eventManager->TriggerEvent2("onStreamSetUnload", {}, setName);
	});

	if (shouldUseMapStore)
	{
		streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
	}
}

static bool LoadtreamSet(const std::string& setName)
{
	static auto resourceManager = Instance<fx::ResourceManager>::Get();
	static auto eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();
	static auto netLibraryResourcesComponent = s_netLibrary->GetComponent<NetLibraryResourcesComponent>();

	std::vector<bool> results;

	resourceManager->ForAllResources([&](fwRefContainer<fx::Resource> resource)
	{
		fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

		auto view = metaData->GetEntries("stream_set");

		for (auto it = view.begin(), end = view.end(); it != end; ++it)
		{
			auto& setName2 = it->second;

			if (setName2 == setName)
			{
				std::string overlayFile = fmt::sprintf("%s.rpf", setName);
				auto entryList = resource->GetComponent<ResourceCacheEntryList>();

				if (entryList->GetEntry(overlayFile))
				{
					bool result = netLibraryResourcesComponent->RequestResourceFileSet(resource.GetRef(), setName);
					results.emplace_back(result);
				}
			}
		}
	});

	bool result = std::all_of(results.begin(), results.end(), [](bool b)
	{
		return b;
	});

	if (result)
	{
		LoadStreamSetInternal(setName);
	}

	return result;
}

static void ReleaseStreamSet(const std::string& setName)
{
	static auto resourceManager = Instance<fx::ResourceManager>::Get();
	static auto eventManager = resourceManager->GetComponent<fx::ResourceEventManagerComponent>();
	static auto netLibraryResourcesComponent = s_netLibrary->GetComponent<NetLibraryResourcesComponent>();

	ReleaseStreamSetInternal(setName);

	resourceManager->ForAllResources([&](fwRefContainer<fx::Resource> resource)
	{
		fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

		auto view = metaData->GetEntries("stream_set");

		for (auto it = view.begin(), end = view.end(); it != end; ++it)
		{
			auto& setName2 = it->second;

			if (setName2 == setName)
			{
				std::string overlayFile = fmt::sprintf("%s.rpf", setName);

				auto entryList = resource->GetComponent<ResourceCacheEntryList>();

				if (entryList->GetEntry(overlayFile))
				{
					netLibraryResourcesComponent->ReleaseResourceFileSet(resource.GetRef(), setName);
				}
			}
		}
	});
}

static HookFunction hookFunction([]()
{
	uint8_t* ptr = (uint8_t*)hook::get_pattern("E8 ? ? ? ? 48 8D 0D ? ? ? ? 33 D2 E8 ? ? ? ? 48 8D 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 45 33 F6");
	s_strStreamingLoaderMgr = (decltype(s_strStreamingLoaderMgr))(ptr + *(int*)(ptr - 4));
	
	ptr += 5;
	FlushStreamingLoader = (decltype(FlushStreamingLoader))(ptr + *(int*)(ptr - 4));

	ptr = (uint8_t*)hook::get_pattern("48 8B CF E8 ? ? ? ? BE ? ? ? ? 45 33 C9");
	s_strStreamingInfomgr = (decltype(s_strStreamingInfomgr))(ptr + *(int*)(ptr - 4));

	ptr = (uint8_t*)hook::get_pattern("E8 ? ? ? ? 48 8D 0D ? ? ? ? E8 ? ? ? ? 8B D6", 5);
	PurgeInfoRequestList = (decltype(PurgeInfoRequestList))(ptr + *(int*)(ptr - 4));

	ptr = (uint8_t*)hook::get_pattern("E8 ? ? ? ? 44 38 7B ? 74 ? 48 8D 0D ? ? ? ? 41 8B D6", 5);
	FlushInfoLoadedList = (decltype(FlushInfoLoadedList))(ptr + *(int*)(ptr - 4));

	// Fix platerwitch stuck on outro hold for too long
	ptr = (uint8_t*)hook::get_pattern("40 88 37 44 88 64 24");
	hook::nop(ptr, 3);
});

static InitFunction initFunction([]()
{
	OnNetBindingsAttachToObject.Connect([](NetLibrary* netLibrary)
	{
		s_netLibrary = netLibrary;
	});
	
	fx::Resource::OnInitializeInstance.Connect([](fx::Resource* resource)
	{
		resource->OnStart.Connect([=]()
		{
			static auto str = streaming::Manager::GetInstance();

			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

			auto view = metaData->GetEntries("stream_set");

			for (const auto& entry : view)
			{
				auto& setName = entry.second;
				auto it2 = s_StreamSetResources.find(setName);

				if (it2 == s_StreamSetResources.end())
				{
					s_StreamSetResources[setName] = {};
				}

				s_StreamSetResources[setName].emplace(resource);
			}
		});

		resource->OnStop.Connect([=]()
		{
			fwRefContainer<fx::ResourceMetaDataComponent> metaData = resource->GetComponent<fx::ResourceMetaDataComponent>();

			auto view = metaData->GetEntries("stream_set");

			for (const auto& entry : view)
			{
				const auto& setName = entry.second;
				auto it2 = s_StreamSetResources.find(setName);

				if (it2 != s_StreamSetResources.end())
				{
					it2->second.erase(resource);

					if (it2->second.empty())
					{
						s_StreamSetResources.erase(it2);
					}
				}
			}
		});
	});

	fx::ScriptEngine::RegisterNativeHandler("REQUEST_STREAM_SET", [](fx::ScriptContext& context)
	{
		std::string setName = context.CheckArgument<const char*>(0);
		bool result = LoadtreamSet(setName);
		context.SetResult(result);
	});

	fx::ScriptEngine::RegisterNativeHandler("RELEASE_STREAM_SET", [](fx::ScriptContext& context)
	{
		std::string setName = context.CheckArgument<const char*>(0);
		ReleaseStreamSet(setName);
	});
});

#endif
