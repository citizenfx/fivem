#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <ICoreGameInit.h>
#include <Streaming.h>
#include <EntitySystem.h>

#include <Local.h>

#include <algorithm>
#include <mutex>
#include <unordered_map>
#include <unordered_set>
#include <vector>

constexpr size_t kExtendedDependencyCount = 256;
constexpr int kResidentStreamingFlags = 7;
constexpr int kResidentRequiredFlags = 3;

constexpr uint32_t kStatusMask = 3;
constexpr uint32_t kStatusLoaded = 1;
constexpr uint32_t kRequiredFlagsMask = 0x00F10000;
constexpr uint32_t kDependentShift = 2;
constexpr uint32_t kDependentMask = 0x3FFF;

static void (*g_origSetPedMetaDataFile)(void*, uint32_t);
static int (*g_origGetDependencies)(void*, uint32_t, uint32_t*, int);

static streaming::strStreamingModule* g_metaDataStore;

static std::mutex g_residentMutex;
static std::unordered_set<uint32_t> g_residentIndices;

struct ResidentOwner
{
	uint32_t index;
	uint32_t hash;
};

static std::unordered_map<uint32_t, std::vector<ResidentOwner>> g_residentOwners;

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

		manager->ReleaseObject(index, kResidentRequiredFlags);
	}
}

static bool IsOwnerBlockingReload(streaming::Manager* manager, const ResidentOwner& owner, uint32_t globalIndex)
{
	if ((GetEntryFlags(manager, owner.index) & kStatusMask) != 0)
	{
		return true;
	}

	auto module = rage::fwArchetypeManager::GetStreamingModule();
	rage::fwModelId modelId;

	if (!rage::fwArchetypeManager::GetArchetypeFromHashKey(owner.hash, modelId) || module->baseIdx + modelId.modelIndex != owner.index)
	{
		return false;
	}

	uint32_t dependencies[kExtendedDependencyCount];
	auto end = dependencies + kExtendedDependencyCount;

	if (g_origGetDependencies(module, modelId.modelIndex, dependencies, static_cast<int>(kExtendedDependencyCount)) < static_cast<int>(kExtendedDependencyCount))
	{
		return false;
	}

	return std::find(dependencies, end, globalIndex) == end;
}

bool ReloadOverriddenMetaData(uint32_t globalIndex, uint32_t nameHash)
{
	auto manager = streaming::Manager::GetInstance();

	if (!manager || !manager->Entries || globalIndex >= static_cast<uint32_t>(manager->numEntries))
	{
		return false;
	}

	std::vector<ResidentOwner> owners;

	{
		std::lock_guard<std::mutex> lock(g_residentMutex);

		auto it = g_residentOwners.find(globalIndex);

		if (it == g_residentOwners.end() || g_residentIndices.find(globalIndex) == g_residentIndices.end())
		{
			return false;
		}

		owners = it->second;
	}

	uint32_t flags = manager->Entries[globalIndex].flags;

	if ((flags & kStatusMask) != kStatusLoaded || (flags & kRequiredFlagsMask) == 0 || ((flags >> kDependentShift) & kDependentMask) != 0)
	{
		return false;
	}

	rage::fwModelId modelId;

	if (rage::fwArchetypeManager::GetArchetypeFromHashKey(nameHash, modelId))
	{
		owners.push_back({ rage::fwArchetypeManager::GetStreamingModule()->baseIdx + modelId.modelIndex, nameHash });
	}

	for (const auto& owner : owners)
	{
		if (IsOwnerBlockingReload(manager, owner, globalIndex))
		{
			return false;
		}
	}


	manager->ReleaseObject(globalIndex, kResidentRequiredFlags);
	manager->ReleaseObject(globalIndex);
	manager->RequestObject(globalIndex, kResidentStreamingFlags);

	return true;
}

static void SetPedMetaDataFile(void* self, uint32_t localIndex)
{
	g_origSetPedMetaDataFile(self, localIndex);

	if (localIndex != -1)
	{
		if (auto store = GetMetaDataStore())
		{
			auto archetype = static_cast<rage::fwArchetype*>(self);
			uint32_t globalIndex = store->baseIdx + localIndex;
			uint32_t ownerIndex = rage::fwArchetypeManager::GetStreamingModule()->baseIdx + archetype->assignedStreamingSlot;

			{
				std::lock_guard<std::mutex> lock(g_residentMutex);
				auto& owners = g_residentOwners[globalIndex];
				auto it = std::find_if(owners.begin(), owners.end(), [ownerIndex](const ResidentOwner& owner)
				{
					return owner.index == ownerIndex;
				});

				if (it == owners.end())
				{
					owners.push_back({ ownerIndex, archetype->hash });
				}
				else
				{
					it->hash = archetype->hash;
				}
			}

			EnsureObjectResident(globalIndex);
		}
	}
}

static int GetDependencies(void* self, uint32_t localIndex, uint32_t* outIndices, int count)
{
	auto manager = streaming::Manager::GetInstance();

	if (count <= 0 || static_cast<size_t>(count) >= kExtendedDependencyCount || !manager || !manager->Entries)
	{
		return g_origGetDependencies(self, localIndex, outIndices, count);
	}

	uint32_t dependencies[kExtendedDependencyCount];

	int total = std::clamp(g_origGetDependencies(self, localIndex, dependencies, kExtendedDependencyCount), 0, static_cast<int>(kExtendedDependencyCount));

	if (total <= count)
	{
		memcpy(outIndices, dependencies, total * sizeof(uint32_t));
		return total;
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

static HookFunction hookFunction([]
{
	g_origSetPedMetaDataFile = hook::trampoline(hook::get_pattern("83 FA FF 74 ? 53 48 83 EC 20 8B DA 48 81 C1 E0 00 00 00 BA 01 00 00 00 E8"), SetPedMetaDataFile);
	g_origGetDependencies = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 44 89 48 ? 89 50 ? 48 89 48"), GetDependencies);

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]
	{
		ReleaseResidentObjects();
	}, -10000);
});
