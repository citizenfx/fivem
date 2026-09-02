#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <ICoreGameInit.h>
#include <Streaming.h>

#include <mutex>
#include <unordered_set>

constexpr size_t kExtendedDependencyCount = 256;
constexpr int kResidentStreamingFlags = 7;

constexpr uint32_t kStatusMask = 3;
constexpr uint32_t kStatusLoaded = 1;
constexpr uint32_t kRequiredFlagsMask = 0x00F10000;

static void (*g_origSetPedMetaDataFile)(void*, uint32_t);
static int (*g_origGetDependencies)(void*, uint32_t, uint32_t*, int);

static streaming::strStreamingModule* g_metaDataStore;

static std::mutex g_residentMutex;
static std::unordered_set<uint32_t> g_residentIndices;

static streaming::strStreamingModule* GetMetaDataStore()
{
	if (!g_metaDataStore)
	{
		if (auto manager = streaming::Manager::GetInstance())
		{
			g_metaDataStore = manager->moduleMgr.GetStreamingModule("ymt");
		}
	}

	return g_metaDataStore;
}

static uint32_t GetEntryFlags(streaming::Manager* manager, uint32_t globalIndex)
{
	return (globalIndex < static_cast<uint32_t>(manager->numEntries)) ? manager->Entries[globalIndex].flags : 0;
}

static void EnsureObjectResident(uint32_t globalIndex)
{
	auto manager = streaming::Manager::GetInstance();

	if (!manager || !manager->Entries || globalIndex >= static_cast<uint32_t>(manager->numEntries))
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(g_residentMutex);
		g_residentIndices.insert(globalIndex);
	}

	if ((GetEntryFlags(manager, globalIndex) & kRequiredFlagsMask) == 0)
	{
		manager->RequestObject(globalIndex, kResidentStreamingFlags);
	}
}

static void ReleaseResidentObjects()
{
	std::unordered_set<uint32_t> indices;

	{
		std::lock_guard<std::mutex> lock(g_residentMutex);
		indices.swap(g_residentIndices);
	}

	auto manager = streaming::Manager::GetInstance();

	if (!manager || !manager->Entries)
	{
		return;
	}

	for (uint32_t index : indices)
	{
		if (index >= static_cast<uint32_t>(manager->numEntries))
		{
			continue;
		}

		if ((manager->Entries[index].flags & kRequiredFlagsMask) == 0)
		{
			manager->RequestObject(index, kResidentStreamingFlags);
		}

		manager->ReleaseObject(index, kResidentStreamingFlags);
	}
}

static void SetPedMetaDataFile(void* self, uint32_t localIndex)
{
	g_origSetPedMetaDataFile(self, localIndex);

	if (localIndex != -1)
	{
		if (auto store = GetMetaDataStore())
		{
			EnsureObjectResident(store->baseIdx + localIndex);
		}
	}
}

static int __declspec(noinline) FilterDependencies(const uint32_t* dependencies, int total, uint32_t* outIndices, int count)
{
	auto manager = streaming::Manager::GetInstance();

	if (!manager || !manager->Entries)
	{
		memcpy(outIndices, dependencies, static_cast<size_t>(count) * sizeof(uint32_t));
		return count;
	}

	uint32_t rearm[kExtendedDependencyCount];
	int rearmCount = 0;
	int written = 0;
	int index = 0;

	{
		std::lock_guard<std::mutex> lock(g_residentMutex);

		for (; index < total && written < count; index++)
		{
			uint32_t dependency = dependencies[index];

			if (g_residentIndices.find(dependency) != g_residentIndices.end())
			{
				uint32_t flags = GetEntryFlags(manager, dependency);

				if ((flags & kRequiredFlagsMask) == 0)
				{
					rearm[rearmCount++] = dependency;
				}

				if ((flags & kStatusMask) == kStatusLoaded)
				{
					continue;
				}
			}

			outIndices[written++] = dependency;
		}
	}

	for (int i = 0; i < rearmCount; i++)
	{
		manager->RequestObject(rearm[i], kResidentStreamingFlags);
	}

	for (; index < total; index++)
	{
		EnsureObjectResident(dependencies[index]);
	}

	return written;
}

static int GetDependencies(void* self, uint32_t localIndex, uint32_t* outIndices, int count)
{
	if (count <= 0 || static_cast<size_t>(count) >= kExtendedDependencyCount)
	{
		return g_origGetDependencies(self, localIndex, outIndices, count);
	}

	uint32_t dependencies[kExtendedDependencyCount];

	int total = g_origGetDependencies(self, localIndex, dependencies, static_cast<int>(kExtendedDependencyCount));

	if (total == 1)
	{
		outIndices[0] = dependencies[0];
		return 1;
	}

	if (total <= 0)
	{
		return 0;
	}

	if (static_cast<size_t>(total) > kExtendedDependencyCount)
	{
		total = static_cast<int>(kExtendedDependencyCount);
	}

	if (total > count)
	{
		return FilterDependencies(dependencies, total, outIndices, count);
	}

	memcpy(outIndices, dependencies, static_cast<size_t>(total) * sizeof(uint32_t));
	return total;
}

static HookFunction hookFunction([]
{
	g_origSetPedMetaDataFile = hook::trampoline(hook::get_pattern("83 FA FF 74 ? 53 48 83 EC 20 8B DA 48 81 C1 E0 00 00 00 BA 01 00 00 00 E8"), SetPedMetaDataFile);

	auto vtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8D 05 ? ? ? ? 48 8D 0D ? ? ? ? 48 89 05 ? ? ? ? E8 ? ? ? ? 48 8D 15 ? ? ? ? 48 8D 0D ? ? ? ? E8", 3));
	auto slot = *reinterpret_cast<uint32_t*>(hook::get_pattern("FF 90 ? ? ? ? 33 D2 4C 63 C0 85 C0", 2)) / 8;

	g_origGetDependencies = reinterpret_cast<decltype(g_origGetDependencies)>(vtable[slot]);
	hook::putVP(&vtable[slot], reinterpret_cast<uintptr_t>(GetDependencies));

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]
	{
		ReleaseResidentObjects();
	}, -10000);
});
