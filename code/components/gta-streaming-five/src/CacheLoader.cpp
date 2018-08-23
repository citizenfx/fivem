#include "StdInc.h"
#include "Hooking.h"

#include <CoreConsole.h>

#include <atHashMap.h>

#include <Streaming.h>

#include <VFSManager.h>

int GetCollectionIndexByTag(const std::string& tag);
StreamingPackfileEntry* GetStreamingPackfileByTag(const std::string& tag);
void InvalidateCacheByTag(const std::string& tag);

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

struct
{
	struct
	{
		void(*loadCache)();
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

extern std::unordered_map<int, std::string> g_handlesToTag;

static int* GetCacheIndex(int* outPtr, int handle)
{
	int* origRet = g_origGetCacheIndex(outPtr, handle);

	auto it = g_handlesToTag.find(handle);

	if (it != g_handlesToTag.end())
	{
		*outPtr = GetCollectionIndexByTag(it->second);
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
		retval = GetStreamingPackfileByTag(it->second);
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
});

void LoadCache(const char* tagName)
{
	int index = GetCollectionIndexByTag(tagName);

	if (index < 0)
	{
		return;
	}

	// prepare the cache data set
	atHashMap<bool> packfileMap;
	atHashMap_bool_insert(&packfileMap, index, true);

	_loadCacheFile(tagName, va("resources:/%s/", tagName), true, packfileMap);

	if (!_parseCache())
	{
		InvalidateCacheByTag(tagName);
	}
}

uint32_t GetPackHashByTag(const std::string& tag);

static InitFunction initFunction([]()
{
	static ConsoleCommand saveCacheCmd("save_gta_cache", [](const std::string& resourceName)
	{
		int index = GetCollectionIndexByTag(resourceName);

		if (index < 0)
		{
			console::PrintError("cacheloader", "No such collection tag %s\n", resourceName);
			return;
		}

		// prepare the cache data set
		atHashMap<bool> packfileMap;
		atHashMap_bool_insert(&packfileMap, index, true);

		_loadCacheFile(resourceName.c_str(), va("cache:/%s/", resourceName), true, packfileMap);

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
		fileData << fmt::sprintf("%u %lld", HashString(va("resource_surrogate:/%s.rpf", resourceName)), GetPackHashByTag(resourceName)) << "\n";
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

		g_appendToCache = decltype(g_appendToCache)();

		std::string outData = fileData.str();

		stream.Write(outData.c_str(), outData.size());

		trace("Saved cache to %s\n", outFileName);
	});
});

