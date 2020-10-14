#include "StdInc.h"
#include "Hooking.h"

#include <CoreConsole.h>

#include <atHashMap.h>

#include <Streaming.h>

#include <MinHook.h>

#include <VFSManager.h>

#include <gameSkeleton.h>

#include <ResourceCache.h>

#include <Resource.h>
#include <ResourceManager.h>
#include <ResourceMounter.h>

static hook::cdecl_stub<void(atHashMap<bool>*, const uint32_t&, const bool&)> atHashMap_bool_insert([]()
{
	return hook::get_call(hook::get_pattern("49 8B CF C6 45 60 01 E8 ? ? ? ? 0F B7", 7));
});

static hook::cdecl_stub<void(const char* filenameBase, const char* rootPath, int isDlc, const atHashMap<bool>& enabledDLC)> _loadCacheFile([]()
{
	return hook::get_pattern("B9 00 00 04 00 BF 01 00 00 00 39", -0x5D);
});

static hook::cdecl_stub<bool()> _parseCache([]()
{
	return hook::get_pattern("74 1B E8 ? ? ? ? 3B 05", -0x16);
});

extern atArray<StreamingPackfileEntry>* g_streamingPackfiles;

static void(*g_loadCacheOld)(const char* filenameBase, const char* rootPath, int isDlc, const atHashMap<bool>& enabledDLC);

static void LoadCacheHook(const char* filenameBase, const char* rootPath, int isDlc, const atHashMap<bool>& enabledDLC)
{
	if (!isDlc)
	{
		for (auto& file : *g_streamingPackfiles)
		{
			if (file.isDLC)
			{
				file.isDLC = false;

				file.cacheFlags &= ~1;
			}
		}
	}

	g_loadCacheOld(filenameBase, rootPath, isDlc, enabledDLC);
}

struct
{
	struct
	{
		void(*loadCache)(const void* cacheEntry);
		void(*saveCache)();
		char name[48];
		int size;
	} fields[6];

	uint32_t numModules;
}* g_cacheMgr;

static std::function<void(const void*, int)> g_appendToCache;

static void AppendToCacheWrap(const void* data, int length)
{
	if (g_appendToCache)
	{
		g_appendToCache(data, length);
	}
}

static int*(*g_origGetCacheIndex)(int*, int);

extern std::unordered_map<int, std::tuple<std::string, std::string>> g_handlesToTag;

namespace std
{
template<>
struct hash<tuple<string, string>>
{
	size_t operator()(tuple<string, string> const& arg) const noexcept
	{
		return (hash<string>()(std::get<0>(arg)) * 472349) + hash<string>()(std::get<1>(arg));
	}
};
}

static bool g_tagMode;
static std::mutex g_dummyPackfileMutex;
static std::unordered_map<std::string, std::shared_ptr<StreamingPackfileEntry>> g_dummyPackfiles;
static std::unordered_map<std::tuple<std::string, std::string>, std::shared_ptr<StreamingPackfileEntry>> g_dummyPackfilesForTagMode;

static StreamingPackfileEntry* GetDummyStreamingPackfileByTag(const std::tuple<std::string, std::string>& tag)
{
	std::unique_lock<std::mutex> lock(g_dummyPackfileMutex);

	if (g_tagMode)
	{
		auto it = g_dummyPackfilesForTagMode.find(tag);

		if (it == g_dummyPackfilesForTagMode.end())
		{
			auto entry = std::make_shared<StreamingPackfileEntry>();
			memset(&*entry, 0, sizeof(StreamingPackfileEntry));
			entry->cacheFlags = 1;
			entry->nameHash = HashString(fmt::sprintf("resource_surrogate:/%s/%s", std::get<0>(tag), std::get<1>(tag)).c_str());

			it = g_dummyPackfilesForTagMode.emplace(tag, entry).first;
		}

		return &*it->second;
	}

	// try getting data from tagmode at least
	{
		auto it = g_dummyPackfilesForTagMode.find(tag);

		if (it != g_dummyPackfilesForTagMode.end())
		{
			return &*it->second;
		}
	}

	auto it = g_dummyPackfiles.find(std::get<0>(tag));

	if (it == g_dummyPackfiles.end())
	{
		auto entry = std::make_shared<StreamingPackfileEntry>();
		memset(&*entry, 0, sizeof(StreamingPackfileEntry));
		entry->cacheFlags = 1;
		entry->nameHash = HashString(fmt::sprintf("resource_surrogate:/%s.rpf", std::get<0>(tag)).c_str());

		it = g_dummyPackfiles.insert({ std::get<0>(tag), entry }).first;
	}

	return &*it->second;
}

int GetDummyCollectionIndexByTag(const std::tuple<std::string, std::string>& tag)
{
	auto pf = GetDummyStreamingPackfileByTag(tag);

	return (pf->nameHash & 0x7FFFFFFF) | 0x40000000;
}

static void InvalidateDummyCacheByTag(const std::tuple<std::string, std::string>& tag)
{
	auto pf = GetDummyStreamingPackfileByTag(tag);
	pf->cacheFlags |= 1;
}

static void ValidateDummyCacheByTag(const std::tuple<std::string, std::string>& tag)
{
	auto pf = GetDummyStreamingPackfileByTag(tag);
	pf->cacheFlags &= ~1;
}

static int* GetCacheIndex(int* outPtr, int handle)
{
	int* origRet = g_origGetCacheIndex(outPtr, handle);

	auto it = g_handlesToTag.find(handle);

	if (it != g_handlesToTag.end())
	{
		*outPtr = GetDummyCollectionIndexByTag(it->second);
	}

	return outPtr;
}

static StreamingPackfileEntry*(*g_origGetPackIndex)(int);

static StreamingPackfileEntry* GetPackIndex(int handle)
{
	auto retval = g_origGetPackIndex(handle);

	auto it = g_handlesToTag.find(handle);

	if (it != g_handlesToTag.end())
	{
		retval = GetDummyStreamingPackfileByTag(it->second);
	}

	if (retval == nullptr)
	{
		static StreamingPackfileEntry fakePfe;
		fakePfe.cacheFlags = 0;

		retval = &fakePfe;
	}

	return retval;
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern("33 D2 45 85 DB 7E 4D 48 8D 2D", 10);
		g_cacheMgr = hook::get_address<decltype(g_cacheMgr)>(location);
	}

	// append to cache
	hook::jump(hook::get_pattern("B9 CF 66 9F 1F", -0x3F), AppendToCacheWrap);

	// allow collision bounds to be reconfigured even if fwBoxStreamer has already been set up
	// given how collision files won't change at runtime (or at least, not without calling fwBoxStreamer::m_10 first)
	// this should be safe. unless, of course, fwBoxStreamer reorders this array.
	hook::nop(hook::get_pattern("80 B9 0F 01 00 00 00 75 40 4C 8B", 7), 2);

	{
		hook::set_call(&g_origGetCacheIndex, hook::get_pattern("48 8D 4D 20 8B D0 E8 ? ? ? ? 44 8B 00 0F", 6));
		hook::call(hook::get_pattern("48 8D 4D 30 8B D0 E8 ? ? ? ? 44 8B 00 0F", 6), GetCacheIndex);
		hook::call(hook::get_pattern("48 8D 4D 20 8B D0 E8 ? ? ? ? 44 8B 00 0F", 6), GetCacheIndex);
		hook::call(hook::get_pattern("48 8D 4D 10 8B D0 E8 ? ? ? ? 45", 6), GetCacheIndex);
		hook::call(hook::get_pattern("48 8D 8C 24 90 00 00 00 8B D0 E8", 10), GetCacheIndex);

		// CInteriorProxy module
		hook::call(hook::get_pattern("48 8D 8C 24 C0 00 00 00 8B D0 E8", 10), GetCacheIndex); // save
		hook::call(hook::get_pattern("48 8D 8D 88 00 00 00 8B D0 E8", 9), GetCacheIndex); // load
	}
	
	{
		hook::set_call(&g_origGetPackIndex, hook::get_pattern("E8 ? ? ? ? 45 33 FF 40 8A 78 48 40 F6", 0));
		hook::call(hook::get_pattern("E8 ? ? ? ? 45 33 FF 40 8A 78 48 40 F6", 0), GetPackIndex);
		hook::call(hook::get_pattern("E8 ? ? ? ? 8B C8 E8 ? ? ? ? 83 3D ? ? ? ? 01 75", 7), GetPackIndex);
		hook::call(hook::get_pattern("E8 ? ? ? ? 48 85 C0 74 07 8A 40 48", 0), GetPackIndex);
		hook::call(hook::get_pattern("8B C8 E8 ? ? ? ? 45 33 E4 40 8A 78 48", 2), GetPackIndex); // CInteriorProxy load
	}

	MH_Initialize();
	MH_CreateHook(hook::get_pattern("B9 00 00 04 00 BF 01 00 00 00 39", -0x5D), LoadCacheHook, (void**)& g_loadCacheOld);
	MH_EnableHook(MH_ALL_HOOKS);
});

extern std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;

static void SetupCache(const std::tuple<std::string, std::string>& tagple)
{
	auto pf = GetDummyStreamingPackfileByTag(tagple);
	pf->cacheFlags = 0;

	// prepare the cache data set
	int index = GetDummyCollectionIndexByTag(tagple);

	atHashMap<bool> packfileMap;
	atHashMap_bool_insert(&packfileMap, index, true);

	_loadCacheFile(std::get<0>(tagple).c_str(), "dummy_device:/", true, packfileMap);
}

static void LoadSingleCache(const std::string& tagName, const std::string& assetName, const std::string& cacheData, const char* moduleName)
{
	// set up cacheloader
	g_tagMode = true;

	auto tagple = std::make_tuple(tagName, assetName);
	SetupCache(tagple);

	for (int mi = 0; mi < g_cacheMgr->numModules; mi++)
	{
		auto module = &g_cacheMgr->fields[mi];

		if (_stricmp(module->name, moduleName) == 0)
		{
			module->loadCache(cacheData.data());
			break;
		}
	}

	ValidateDummyCacheByTag(tagple);

	g_tagMode = false;
}

static std::string SaveSingleCache(const std::string& tagName, const std::string& assetName, const char* moduleName)
{
	g_tagMode = true;

	std::stringstream cacheData;

	g_appendToCache = [&](const void* data, int length)
	{
		cacheData << std::string((const char*)data, length);
	};

	auto tagple = std::make_tuple(tagName, assetName);
	SetupCache(tagple);

	for (int mi = 0; mi < g_cacheMgr->numModules; mi++)
	{
		auto module = &g_cacheMgr->fields[mi];

		if (_stricmp(module->name, moduleName) == 0)
		{
			module->saveCache();
			break;
		}
	}

	g_appendToCache = {};

	g_tagMode = false;

	return cacheData.str();
}

static std::set<std::string> g_cacheTags;

void LoadCache(const char* tagName)
{
	int index = GetDummyCollectionIndexByTag({ tagName, "" });

	if (index < 0)
	{
		return;
	}

	// load autocache
	static auto resman = Instance<fx::ResourceManager>::Get();

	if (auto resource = resman->GetResource(tagName, false); resource.GetRef())
	{
		auto mounter = resource->GetComponent<fx::ResourceMounter>();

		if (auto rescache = mounter->GetResourceCache(); rescache)
		{
			auto resList = resource->GetComponent<ResourceCacheEntryList>();

			for (const auto& [ name, data ] : resList->GetEntries())
			{
				if (!boost::algorithm::ends_with(name, ".ybn") &&
					!boost::algorithm::ends_with(name, ".ymap"))
				{
					continue;
				}

				const auto& hash = data.referenceHash;

				auto cacheData = rescache->GetCustomEntry(fmt::sprintf("gta_cache:%s", hash));
				
				const char* moduleName = nullptr;

				if (boost::algorithm::ends_with(name, ".ybn"))
				{
					moduleName = "BoundsStore";
				}
				else if (boost::algorithm::ends_with(name, ".ymap"))
				{
					moduleName = "fwMapDataStore";
				}
				
				if (cacheData)
				{
					LoadSingleCache(tagName, data.basename, *cacheData, moduleName);
				}
			}
		}
	}

	// prepare the cache data set
	atHashMap<bool> packfileMap;
	atHashMap_bool_insert(&packfileMap, index, true);

	_loadCacheFile(tagName, va("resources:/%s/", tagName), true, packfileMap);

	if (!_parseCache())
	{
		InvalidateDummyCacheByTag({ tagName, "" });
	}
	else
	{
		ValidateDummyCacheByTag({ tagName, "" });
	}

	g_cacheTags.insert(tagName);
}

static void SaveAllCaches()
{
	for (const auto& tagName : g_cacheTags)
	{
		static auto resman = Instance<fx::ResourceManager>::Get();

		if (auto resource = resman->GetResource(tagName, false); resource.GetRef())
		{
			auto mounter = resource->GetComponent<fx::ResourceMounter>();

			if (auto rescache = mounter->GetResourceCache(); rescache)
			{
				auto resList = resource->GetComponent<ResourceCacheEntryList>();

				for (const auto& [name, data] : resList->GetEntries())
				{
					if (!boost::algorithm::ends_with(name, ".ybn") && !boost::algorithm::ends_with(name, ".ymap"))
					{
						continue;
					}

					const auto& hash = data.referenceHash;

					auto ckey = fmt::sprintf("gta_cache:%s", hash);
					auto cacheData = rescache->GetCustomEntry(ckey);

					const char* moduleName = nullptr;

					if (boost::algorithm::ends_with(name, ".ybn"))
					{
						moduleName = "BoundsStore";
					}
					else if (boost::algorithm::ends_with(name, ".ymap"))
					{
						moduleName = "fwMapDataStore";
					}

					if (!cacheData)
					{
						auto cacheData = SaveSingleCache(tagName, data.basename, moduleName);

						rescache->AddCustomEntry(ckey, cacheData);
					}
				}
			}
		}
	}

	g_cacheTags.clear();
}

uint32_t GetPackHashByTag(const std::string& tag);

static InitFunction initFunction([]()
{
	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			SaveAllCaches();
		}
	}, INT32_MAX);

	static ConsoleCommand saveCacheCmd("save_gta_cache", [](const std::string& resourceName)
	{
		atHashMap<bool> packfileMap;

		if (resourceName != "gta5")
		{
			int index = GetDummyCollectionIndexByTag({ resourceName, "" });

			if (index < 0)
			{
				console::PrintError("cacheloader", "No such collection tag %s\n", resourceName);
				return;
			}

			auto pf = GetDummyStreamingPackfileByTag({ resourceName, "" });
			pf->cacheFlags = 0;

			// prepare the cache data set
			atHashMap_bool_insert(&packfileMap, index, true);

			_loadCacheFile(resourceName.c_str(), va("cache:/%s/", resourceName), true, packfileMap);
		}
		else
		{
			_loadCacheFile("gta5", nullptr, false, packfileMap);
		}

		// save the cache
		std::string outFileName = fmt::sprintf("fxd:/%s_cache_y.dat", resourceName);
		auto device = vfs::GetDevice(outFileName);

		if (!device.GetRef())
		{
			trace("Invalid device. :/\n");
			return;
		}

		auto handle = device->Create(outFileName);

		if (handle == INVALID_DEVICE_HANDLE)
		{
			trace("Invalid device handle. :/\n");
			return;
		}

		vfs::Stream stream(device, handle);

		std::vector<uint8_t> header(100);
		strcpy(reinterpret_cast<char*>(header.data()), "[VERSION]\n46\n");

		stream.Write(header);

		std::stringstream fileData;
		fileData << "<fileDates>\n";

		if (resourceName != "gta5")
		{
			fileData << fmt::sprintf("%u %lld", HashString(va("resource_surrogate:/%s.rpf", resourceName)), 0xDEADBEEF) << "\n";
		}
		else
		{
			for (auto& file : *g_streamingPackfiles)
			{
				fileData << fmt::sprintf("%u %lld\n", file.nameHash, ((uint64_t)file.modificationTime.dwHighDateTime << 32) | file.modificationTime.dwLowDateTime);
			}
		}

		fileData << "</fileDates>\n";

		for (int i = 0; i < g_cacheMgr->numModules; i++)
		{
			std::stringstream cacheData;

			g_appendToCache = [&](const void* data, int length)
			{
				cacheData << std::string((const char*)data, length);
			};

			fileData << "<module>\n" << g_cacheMgr->fields[i].name << "\n";
			g_cacheMgr->fields[i].saveCache();

			// cache is prefixed with size
			std::string cacheStr = cacheData.str();

			uint32_t size = static_cast<uint32_t>(cacheStr.size());

			fileData << std::string(reinterpret_cast<char*>(&size), sizeof(size));

			fileData << cacheStr;
			fileData << "</module>\n";
		}

		g_appendToCache = {};

		std::string outData = fileData.str();

		stream.Write(outData.c_str(), outData.size());

		trace("Saved cache to %s\n", outFileName);
	});
});

