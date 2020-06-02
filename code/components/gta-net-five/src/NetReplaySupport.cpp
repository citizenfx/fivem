#include <StdInc.h>
#include <Hooking.h>

#include <Resource.h>
#include <ResourceManager.h>

#include <CachedResourceMounter.h>
#include <ResourceCache.h>

#include <ICoreGameInit.h>

static uint32_t (*rage__fiFile__Read)(void* file, void* read, uint32_t size);
static uint32_t (*rage__fiFile__Write)(void* file, const void* read, uint32_t size);
static size_t (*rage__fiFile__Seek)(void* file, int64_t offset);

static hook::cdecl_stub<size_t(void* file)> rage__fiFile__Tell([]()
{
	return hook::get_pattern("8B 41 20 03 41 18 C3");
});

static std::vector<char> g_replaySaveBuffer;

struct ResourceMetaData
{
	std::string name;
	std::string uri;
	std::vector<ResourceCacheEntryList::Entry> entries;

	MSGPACK_DEFINE_MAP(name, uri, entries);
};

static int* replayFileCommand;

static void StartLoadResources(const std::vector<ResourceMetaData>& rmds)
{
	auto resman = Instance<fx::ResourceManager>::Get();

	resman->ResetResources();

	// allow subdir entries to work
	Instance<ICoreGameInit>::Get()->SetData("policy", "[subdir_file_mapping]");
	Instance<ICoreGameInit>::Get()->SetVariable("gameKilled");
	Instance<ICoreGameInit>::Get()->ClearVariable("networkInited");

	for (const auto& md : rmds)
	{
		fwRefContainer<fx::CachedResourceMounter> mounter = resman->GetMounterForUri(md.uri);

		if (mounter.GetRef())
		{
			mounter->RemoveResourceEntries(md.name);

			for (auto& entry : md.entries)
			{
				mounter->AddResourceEntry(md.name, entry.basename, entry.referenceHash, entry.remoteUrl, entry.size, entry.extData);
			}

			auto resourceResult = resman->AddResourceWithError(md.uri).get();

			if (resourceResult)
			{
				fwRefContainer<fx::Resource> resource = resourceResult.value();

				resource->Start();

				// add SEDs
				for (auto& rentry : md.entries)
				{
					fx::StreamingEntryData entry;

					if (rentry.extData.find("rscVersion") != rentry.extData.end())
					{
						auto ed = rentry.extData;

						entry.rscVersion = std::stoul(ed["rscVersion"]);
						entry.rscPagesPhysical = std::stoul(ed["rscPagesPhysical"]);
						entry.rscPagesVirtual = std::stoul(ed["rscPagesVirtual"]);

						entry.filePath = mounter->FormatPath(md.name, rentry.basename);
						entry.resourceName = md.name;

						fx::OnAddStreamingResource(entry);
					}
				}
			}
		}
	}

	Instance<ICoreGameInit>::Get()->ClearVariable("gameKilled");
}

static uint32_t ReadHeaderChunkWrap(void* file, void* read, uint32_t size)
{
	auto rv = rage__fiFile__Read(file, read, size);
	auto pos = rage__fiFile__Tell(file);

	uint32_t hdr = 0;
	rage__fiFile__Read(file, &hdr, sizeof(hdr));

	if (hdr == 0xCFCF5555)
	{
		uint32_t size = 0;
		rage__fiFile__Read(file, &size, sizeof(size));

		g_replaySaveBuffer.resize(size);
		rage__fiFile__Read(file, g_replaySaveBuffer.data(), size);
	}
	else
	{
		g_replaySaveBuffer.clear();
		rage__fiFile__Seek(file, pos);
	}

	return rv;
}

static uint32_t WriteHeaderChunkWrap(void* file, const void* write, uint32_t size)
{
	auto rv = rage__fiFile__Write(file, write, size);

	uint32_t magic = 0xCFCF5555;
	rage__fiFile__Write(file, &magic, sizeof(magic));

	auto resman = Instance<fx::ResourceManager>::Get();

	std::vector<ResourceMetaData> buffer;

	resman->ForAllResources([&buffer](const fwRefContainer<fx::Resource>& resource)
	{
		auto resourceList = resource->GetComponent<ResourceCacheEntryList>();

		ResourceMetaData md;
		md.name = resource->GetName();
		md.uri = resourceList->GetInitUrl();

		for (const auto& [key, entry] : resourceList->GetEntries())
		{
			md.entries.push_back(entry);
		}

		buffer.push_back(std::move(md));
	});

	msgpack::sbuffer sb;
	msgpack::pack(sb, buffer);

	uint32_t wsize = sb.size();
	rage__fiFile__Write(file, &wsize, sizeof(wsize));

	rage__fiFile__Write(file, sb.data(), wsize);

	return rv;
}

static void (*g_origLoadReplayDlc)(void* ecw);

static bool g_lastReplayWasCfx;

static void LoadReplayDlc(void* ecw)
{
	if (!g_replaySaveBuffer.empty())
	{
		auto unpacked = msgpack::unpack(g_replaySaveBuffer.data(), g_replaySaveBuffer.size());
		std::vector<ResourceMetaData> rmds = unpacked.get().as<std::vector<ResourceMetaData>>();

		StartLoadResources(rmds);

		g_lastReplayWasCfx = true;
	}
	else if (g_lastReplayWasCfx)
	{
		g_lastReplayWasCfx = false;

		Instance<fx::ResourceManager>::Get()->ResetResources();
	}

	g_origLoadReplayDlc(ecw);
}

static HookFunction hookFunction([]()
{
	// replay header loading
	{
		auto location = hook::get_pattern<char>("77 B6 48 8B D7 48 8B CB E8", 8);
		hook::set_call(&rage__fiFile__Seek, location - 0x18);
		hook::set_call(&rage__fiFile__Read, location);
		hook::call(location, ReadHeaderChunkWrap);
	}

	// replay header saving
	{
		auto location = hook::get_pattern<char>("48 8B D3 48 8B CF E8 ? ? ? ? 48 8B D7 48 8B CB 44 38", 6);
		hook::set_call(&rage__fiFile__Write, location);
		hook::call(location, WriteHeaderChunkWrap);
	}

	// replay dlc loading (but from here)
	{
		auto location = hook::get_pattern("0F 84 ? ? ? ? 48 8B 0D ? ? ? ? C6 05 ? ? ? ? 01 E8", 20);
		hook::set_call(&g_origLoadReplayDlc, location);
		hook::call(location, LoadReplayDlc);
	}

	// +1 as this is a cmp with imm8
	replayFileCommand = (int*)(hook::get_address<char*>(hook::get_pattern<char>("83 3D ? ? ? ? 03 74 0C 83 3D", 2)) + 1);
});
