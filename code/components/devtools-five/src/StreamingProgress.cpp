#include <StdInc.h>

#include <CoreConsole.h>

#include <Hooking.h>

#include <Streaming.h>
#include <nutsnbolts.h>

#include <StreamingEvents.h>

#include <shared_mutex>

#include <mmsystem.h>

#include <ResourceCacheDeviceV2.h>
#include <VFSManager.h>

struct StreamingDownloadProgress
{
	size_t downloadDone;
	size_t downloadTotal;
};

static std::shared_mutex g_mutex;
static std::map<std::string, StreamingDownloadProgress> g_downloadProgress;
static std::map<std::string, size_t> g_downloadList;
static std::set<std::string> g_downloadDone;
static std::map<std::string, std::string> g_nameMap;

static uint32_t g_lastDownloadTime;

static void StreamingProgress_OnDownload(const std::string& fileName, size_t done, size_t total)
{
	std::unique_lock<std::shared_mutex> lock(g_mutex);

	auto tgtName = fileName;

	if (fileName.find("cache_nb:/") == 0)
	{
		tgtName = "cache:/" + fileName.substr(10);
	}
	else if (fileName.find("compcache_nb:/") == 0)
	{
		tgtName = "compcache:/" + fileName.substr(14);
	}

	g_downloadProgress[tgtName] = { done, total };

	if (done == total)
	{
		if (g_downloadList.find(tgtName) != g_downloadList.end())
		{
			g_downloadDone.insert(tgtName);
		}
	}
}

static hook::cdecl_stub<rage::fiCollection* ()> getRawStreamer([]()
{
	return hook::get_call(hook::get_pattern("48 8B D3 4C 8B 00 48 8B C8 41 FF 90 ? 01 00 00", -5));
});

static void StreamingProgress_Update()
{
	// process requests
	auto streaming = streaming::Manager::GetInstance();
	int thisRequests = 0;

	std::set<std::string> foundNow;

	for (const auto* entry = streaming->RequestListHead; entry; entry = entry->Next)
	{
		auto data = &streaming->Entries[entry->Index];

		// try getting streaming data
		StreamingPackfileEntry* spf = streaming::GetStreamingPackfileForEntry(data);

		if (spf)
		{
			char nameBuffer[256];

			rage::fiCollection* collection = nullptr;

			if (spf)
			{
				collection = reinterpret_cast<rage::fiCollection*>(spf->packfile);
			}

			if (!collection && (data->handle >> 16) == 0)
			{
				collection = getRawStreamer();
			}

			if (collection)
			{
				// is this a networked packfile?
				if (!spf->isHdd)
				{
					strcpy(nameBuffer, "CfxRequest");

					collection->GetEntryNameToBuffer(data->handle & 0xFFFF, nameBuffer, 255);

					bool isCache = false;
					std::string fileName;
					char readBuffer[2048];

					if (strncmp(nameBuffer, "cache:/", 7) == 0)
					{
						fileName = std::string("cache_nb:/") + &nameBuffer[7];
						isCache = true;
					}

					if (strncmp(nameBuffer, "compcache:/", 11) == 0)
					{
						fileName = std::string("compcache_nb:/") + &nameBuffer[11];
						isCache = true;
					}

					if (isCache)
					{
						auto device = vfs::GetDevice(fileName);
						fwRefContainer<resources::ResourceCacheDeviceV2> resDevice(device);

						if (!resDevice->ExistsOnDisk(fileName))
						{
							std::unique_lock<std::shared_mutex> lock(g_mutex);

							if (g_downloadList.find(nameBuffer) == g_downloadList.end())
							{
								g_downloadList.insert({ nameBuffer, resDevice->GetLength(fileName) });
							}

							foundNow.insert(nameBuffer);

							thisRequests++;
						}
					}
				}
			}
		}
	}

	// remove any entries that are no longer requested
	for (auto it = g_downloadList.begin(); it != g_downloadList.end(); )
	{
		// skip if it's actually done
		if (g_downloadDone.find(it->first) != g_downloadDone.end())
		{
			it++;
			continue;
		}

		// if not found this frame, remove it
		if (foundNow.find(it->first) == foundNow.end())
		{
			std::unique_lock<std::shared_mutex> lock(g_mutex);
			it = g_downloadList.erase(it);
		}
		else
		{
			it++;
		}
	}

	if (thisRequests > 0)
	{
		g_lastDownloadTime = timeGetTime();
	}

	if ((timeGetTime() - g_lastDownloadTime) < 2000)
	{
		std::shared_lock<std::shared_mutex> lock(g_mutex);

		size_t downloadDone = 0;
		size_t downloadSize = 0;

		for (auto& downloadPair : g_downloadList)
		{
			downloadDone += g_downloadProgress[downloadPair.first].downloadDone;
			downloadSize += downloadPair.second;
		}

		std::string str = fmt::sprintf("Downloading assets (%d of %d)... (%.2f/%.2f MB)", std::min(g_downloadDone.size(), g_downloadList.size()), g_downloadList.size(), std::min(downloadDone, downloadSize) / 1024.0 / 1024.0, downloadSize / 1024.0 / 1024.0);

		// 1604
		((void(*)(const char*, int, int))hook::get_adjusted(0x1401C3578))(str.c_str(), 5, 2);
	}
	else
	{
		// 1604
		((void(*)(int))hook::get_adjusted(0x1401C3438))(2);

		g_downloadList.clear();
		g_downloadDone.clear();
	}
}

static InitFunction initFunction([]()
{
	// TODO: KillNetwork handler
	fx::OnCacheDownloadStatus.Connect([](const std::string& fileName, size_t done, size_t total)
	{
		StreamingProgress_OnDownload(fileName, done, total);
	});

	OnMainGameFrame.Connect([]()
	{
		static ConVar<bool> useStreamingProgress("game_showStreamingProgress", ConVar_Archive, false);

		if (useStreamingProgress.GetValue())
		{
			StreamingProgress_Update();
		}
	});
});
