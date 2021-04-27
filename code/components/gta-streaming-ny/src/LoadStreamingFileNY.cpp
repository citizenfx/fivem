#include <StdInc.h>
#include <Hooking.h>

#include <Streaming.h>

#include <nutsnbolts.h>
#include <MinHook.h>

static std::vector<std::tuple<std::string, std::string, rage::ResourceFlags>> g_customStreamingFiles;
std::set<std::string> g_customStreamingFileRefs;
static std::map<std::string, std::vector<std::string>, std::less<>> g_customStreamingFilesByTag;
static bool g_reloadStreamingFiles;

void DLL_EXPORT CfxCollection_AddStreamingFileByTag(const std::string& tag, const std::string& fileName, rage::ResourceFlags flags)
{
	auto baseName = std::string(strrchr(fileName.c_str(), '/') + 1);

	g_customStreamingFilesByTag[tag].push_back(fileName);
	g_customStreamingFiles.push_back({ fileName, tag, flags });
	g_customStreamingFileRefs.insert(baseName);

	g_reloadStreamingFiles = true;
}

void DLL_EXPORT CfxCollection_RemoveStreamingTag(const std::string& tag)
{
}

void DLL_EXPORT CfxCollection_BackoutStreamingTag(const std::string& tag)
{
}

void DLL_EXPORT CfxCollection_SetStreamingLoadLocked(bool locked)
{
}

struct FakeRawStreamer
{
	// if this is big, do proper splitting
	std::vector<std::string> fileNames;

	FakeRawStreamer()
	{
		fileNames.reserve(4096);
	}

	size_t RegisterFile(const std::string& fileName)
	{
		// try finding one
		auto it = std::find(fileNames.begin(), fileNames.end(), fileName);

		if (it != fileNames.end())
		{
			return it - fileNames.begin();
		}

		fileNames.push_back(fileName);
		return fileNames.size() - 1;
	}

	const std::string& GetFile(size_t index)
	{
		return fileNames[index];
	}
};

static FakeRawStreamer rawStreamer;

static std::unordered_map<int, std::list<std::tuple<uint32_t, StreamHandle>>> g_handleStack;

static StreamHandle GetRawHandle(size_t index)
{
	StreamHandle h;
	h.imgIndex = 0xFE;
	h.offset = 0xDEADED;
	h.size = index + 1;
	h.unk = 0;
	return h;
}

static std::set<int> myidx;

void LoadStreamingFiles()
{
	auto infoMgr = CStreamingInfoManager::GetInstance();

	for (auto it = g_customStreamingFiles.begin(); it != g_customStreamingFiles.end();)
	{
		auto [file, tag, flags] = *it;
		it = g_customStreamingFiles.erase(it);

		// get basename ('thing.#td') and asset name ('thing')
		const char* slashPos = strrchr(file.c_str(), '/');

		if (slashPos == nullptr)
		{
			continue;
		}

		auto baseName = std::string{ slashPos + 1 };
		auto nameWithoutExt = baseName.substr(0, baseName.find_last_of('.'));

		const char* extPos = strrchr(baseName.c_str(), '.');

		if (extPos == nullptr)
		{
			trace("can't register %s: it doesn't have an extension, why is this in stream/?\n", file);
			continue;
		}

		std::string ext = extPos + 1;

		auto moduleIdx = streamingModuleMgr->GetModuleFromExt(ext.c_str());
		if (moduleIdx == 0xFF)
		{
			trace("can't register %s: no streaming module (does this file even belong in stream?)\n", file);
			continue;
		}

		auto module = &streamingModuleMgr->modules[moduleIdx];
		auto entryIdx = module->getIndexByName(nameWithoutExt.c_str());
		
		// if the asset exists
		auto globalIdx = entryIdx + module->startIndex;
		auto& entry = infoMgr->Entries[globalIdx];
		auto fileIndex = rawStreamer.RegisterFile(file);

		if (entry.handle.offset != 0)
		{
			auto& hs = g_handleStack[globalIdx];

			if (hs.empty())
			{
				hs.push_front({ entry.realSize, entry.handle });
			}

			entry.handle = GetRawHandle(fileIndex);
			entry.realSize = flags;
			hs.push_front({ entry.realSize, entry.handle });
		}
		else
		{
			auto& hs = g_handleStack[globalIdx];
			entry.nextOnCd = -1;
			entry.module = moduleIdx;
			entry.flags = 0;

			entry.moduleByIdx = moduleIdx;
			entry.handle = GetRawHandle(fileIndex);
			entry.realSize = flags;
			hs.push_front({ entry.realSize, entry.handle });
		}

		myidx.insert(globalIdx);

		if (flags & 0x40000000)
		{
			entry.flags |= 0x2000;
		}
		else
		{
			entry.flags &= ~0x2000;
		}
	}
}

/*// start of func
"89 84 24 00 01 00 00 A1 ? ? ? ? 56 57", -13
// call of pgStreamer::Open
"89 47 10 83 F8 FF", -10*/

struct Req
{
	uint32_t a;
	uint32_t b;
	uint16_t idx;
	uint8_t pad[6];
	uint32_t streamHandle;
};

static hook::cdecl_stub<uint32_t(const char* fn, int flag, bool flag2)> rage__pgStreamer__Open([]()
{
	return hook::get_call(hook::get_pattern("89 47 10 83 F8 FF", -10));
});

static bool(__stdcall* g_origOpenStreamHandle)(Req* req);

static bool __stdcall _openStreamHandleHook(Req* req)
{
	if (myidx.find(req->idx) != myidx.end())
	{
		trace("");
	}

	const auto& entry = CStreamingInfoManager::GetInstance()->Entries[req->idx];
	std::string fn;
	if (entry.handle.offset == 0xDEADED)
	{
		fn = rawStreamer.GetFile(entry.handle.size - 1);
	}
	else
	{
		fn = fmt::sprintf("stream:/%u", req->idx);
	}

	auto handle = rage__pgStreamer__Open(fn.c_str(), 0, (entry.flags & 0x2000) != 0);
	req->streamHandle = handle;

	return (handle != -1);
}

static HookFunction hookFunction([]()
{
	MH_Initialize();
	MH_CreateHook(hook::get_pattern("89 84 24 00 01 00 00 A1 ? ? ? ? 56 57", -13), _openStreamHandleHook, (void**)&g_origOpenStreamHandle);
	MH_EnableHook(MH_ALL_HOOKS);

	// don't do img bd/hdd (or layer) check
	{
		auto location = hook::get_pattern<char>("75 0C 66 8B 43 0E");
		hook::nop(location, 2);
		hook::put<uint8_t>(location + 12, 0xEB);
	}
});

static InitFunction initFunction([]()
{
	OnMainGameFrame.Connect([]()
	{
		if (g_reloadStreamingFiles)
		{
			auto infoMgr = CStreamingInfoManager::GetInstance();

			if (!infoMgr->Entries)
			{
				return;
			}

			LoadStreamingFiles();

			g_reloadStreamingFiles = false;
		}
	});
});
