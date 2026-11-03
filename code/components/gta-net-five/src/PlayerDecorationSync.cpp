#include <StdInc.h>

#include <Error.h>
#include <GameInit.h>
#include <ICoreGameInit.h>

#include <Hooking.h>
#include <Hooking.FlexStruct.h>
#include <Hooking.Stubs.h>

#include <CloneManager.h>
#include <netObject.h>
#include <net/NetObjEntityType.h>

#include <PedDecorationSort.h>

#include <algorithm>
#include <array>
#include <memory>
#include <optional>
#include <unordered_map>
#include <unordered_set>

// GH-2010/GH-3092: CPlayerAppearanceDataNode carries player decorations as (collection, preset) hashes instead of the game's bitfield

extern ICoreGameInit* icgi;

extern rage::netObject* GetNetObjectFromEntity(void* entity);

static constexpr int32_t kEmblemTypeTournament = 1;
static constexpr uint32_t kMaxPlayerDecorations = 64;

// clone node can't be > 8191 bits
static constexpr uint32_t kPlayerAppearanceNodeMaxBits = 8184;

static constexpr uint32_t kDecorationListMaxBits = 3712;

// bit count of the list, so the server can find it from the end of the node
static constexpr uint32_t kDecorationListTrailerBits = 16;

struct PlayerDecorationEntry
{
	uint32_t collectionHash = 0;
	uint32_t presetHash = 0;

	PlayerDecorationEntry() = default;

	PlayerDecorationEntry(uint32_t collectionHash, uint32_t presetHash)
		: collectionHash(collectionHash), presetHash(presetHash)
	{
	}

	bool operator==(const PlayerDecorationEntry& other) const
	{
		return collectionHash == other.collectionHash && presetHash == other.presetHash;
	}
};

struct PlayerDecorationList
{
	int32_t m_CrewEmblemVariation = 0;
	std::vector<PlayerDecorationEntry> m_Entries;

	bool operator==(const PlayerDecorationList& other) const
	{
		return m_CrewEmblemVariation == other.m_CrewEmblemVariation && m_Entries == other.m_Entries;
	}
};

class PedDecorationPreset : public hook::FlexStruct
{
public:
	static constexpr size_t kSize = 0x40;

	inline uint32_t GetName()
	{
		return Get<uint32_t>(0x14);
	}
};

class PedDecorationCollection : public hook::FlexStruct
{
public:
	static constexpr size_t kSize = 0xA0;

	inline PedDecorationPreset* GetPreset(uint16_t index)
	{
		return reinterpret_cast<PedDecorationPreset*>(Get<char*>(0x0) + index * PedDecorationPreset::kSize);
	}

	inline uint16_t GetPresetCount()
	{
		return Get<uint16_t>(0x8);
	}

	inline uint32_t GetName()
	{
		return Get<uint32_t>(0x10);
	}

	inline bool IsRequiredForSync()
	{
		return Get<uint8_t>(0x9C) != 0;
	}
};

class PedDecorationManager : public hook::FlexStruct
{
public:
	inline static PedDecorationManager** ms_Instance;
	inline static int32_t kCollectionsOffset;

	inline static std::unordered_set<uint64_t> ms_SyncedPresets;
	inline static std::optional<uint32_t> ms_SyncedPresetsGeneration;

	static PedDecorationManager* GetInstance()
	{
		return *ms_Instance;
	}

	inline PedDecorationCollection* GetCollection(uint16_t index)
	{
		return reinterpret_cast<PedDecorationCollection*>(Get<char*>(kCollectionsOffset) + index * PedDecorationCollection::kSize);
	}

	inline uint16_t GetCollectionCount()
	{
		return Get<uint16_t>(kCollectionsOffset + 8);
	}

	// the serializer lists are rebuilt on every collection (un)mount
	void UpdateSyncedPresets()
	{
		const uint32_t generation = fx::GetOverlaySerializerGeneration();

		if (ms_SyncedPresetsGeneration == generation)
		{
			return;
		}

		ms_SyncedPresets.clear();

		for (uint16_t i = 0; i < GetCollectionCount(); i++)
		{
			auto collection = GetCollection(i);

			if (!collection->IsRequiredForSync())
			{
				continue;
			}

			for (uint16_t j = 0; j < collection->GetPresetCount(); j++)
			{
				ms_SyncedPresets.insert(MakePresetKey(collection->GetName(), collection->GetPreset(j)->GetName()));
			}
		}

		ms_SyncedPresetsGeneration = generation;
	}

	static bool IsSyncedPreset(uint32_t collectionHash, uint32_t presetHash)
	{
		return collectionHash && presetHash && ms_SyncedPresets.find(MakePresetKey(collectionHash, presetHash)) != ms_SyncedPresets.end();
	}

private:
	static inline uint64_t MakePresetKey(uint32_t collectionHash, uint32_t presetHash)
	{
		return (static_cast<uint64_t>(collectionHash) << 32) | presetHash;
	}
};

class CPedDamageBlitDecoration : public hook::FlexStruct
{
public:
	static constexpr size_t kSize = 0x78;

	inline uint32_t GetCollectionName()
	{
		return Get<uint32_t>(0x38);
	}

	inline uint32_t GetPresetName()
	{
		return Get<uint32_t>(0x3C);
	}

	inline int32_t GetEmblemType()
	{
		return Get<int32_t>(0x48);
	}

	inline int32_t GetEmblemId()
	{
		return Get<int32_t>(0x4C);
	}
};

class CPedDamageSet : public hook::FlexStruct
{
public:
	inline static int32_t kDecorationCountOffset;

	inline CPedDamageBlitDecoration* GetDecoration(uint16_t index)
	{
		return reinterpret_cast<CPedDamageBlitDecoration*>(Get<char*>(kDecorationCountOffset - 8) + index * CPedDamageBlitDecoration::kSize);
	}

	inline uint16_t GetDecorationCount()
	{
		return Get<uint16_t>(kDecorationCountOffset);
	}
};

class CPedDamageManager : public hook::FlexStruct
{
public:
	inline static CPedDamageManager** ms_Instance;
	inline static int32_t kDamageSetPoolOffset;
	inline static int32_t kPedDamageSetIdOffset;

	static CPedDamageSet* GetDamageSet(void* ped)
	{
		auto damageSetId = reinterpret_cast<hook::FlexStruct*>(ped)->Get<uint8_t>(kPedDamageSetIdOffset);
		auto instance = *ms_Instance;

		if (damageSetId == 0xFF || !instance)
		{
			return nullptr;
		}

		return instance->Get<CPedDamageSet*>(kDamageSetPoolOffset + damageSetId * sizeof(void*));
	}
};

// rage::CSyncDataBase
class SyncDataSerialiser : public hook::FlexStruct
{
public:
	inline bool IsReader()
	{
		return Get<uint8_t>(8) == 1;
	}

	inline bool IsMaximumSizeSerialiser()
	{
		return CallVirtual<bool>(0xC8);
	}

	inline uint32_t GetSize()
	{
		return CallVirtual<uint32_t>(0xD0);
	}

	inline void SerialiseBool(bool& value)
	{
		CallVirtual<void>(0x38, &value, (const char*)nullptr);
	}

	inline void SerialiseUnsigned(uint32_t& value, int bits)
	{
		CallVirtual<void>(0x68, &value, bits, (const char*)nullptr);
	}
};

static void (*clearCompressedPedDecorations)(PedDecorationManager*, void*);
static bool (*checkPlaceDecoration)(PedDecorationManager*, void*, uint32_t*, uint32_t*);
static bool (*placeDecoration)(PedDecorationManager*, void*, uint32_t*, uint32_t*, int);

static uint32_t (*g_PedDecorationManager_GetPedDecorationCount)(PedDecorationManager*, void*);

static PlayerDecorationList g_NodeDecorations;

static bool g_ApplyingRemoteDecorations;

static uint32_t g_DecorationListMaxBits;

static int32_t g_PlayerAppearanceNodeHasDecorationsOffset;
static int32_t g_PlayerAppearanceAccessorPedOffset;

static bool IsDecorationListSyncActive()
{
	return icgi->OneSyncEnabled && icgi->HasVariable("playerDecorationSyncFix");
}

static void CollectDecorations(void* ped, PlayerDecorationList& list)
{
	list.m_CrewEmblemVariation = 0;
	list.m_Entries.clear();

	auto manager = PedDecorationManager::GetInstance();
	auto damageSet = CPedDamageManager::GetDamageSet(ped);

	if (!manager || !damageSet)
	{
		return;
	}

	manager->UpdateSyncedPresets();

	for (uint16_t i = 0; i < damageSet->GetDecorationCount(); i++)
	{
		auto decoration = damageSet->GetDecoration(i);

		if (decoration->GetEmblemType() == kEmblemTypeTournament)
		{
			list.m_CrewEmblemVariation = decoration->GetEmblemId();
		}

		PlayerDecorationEntry entry{ decoration->GetCollectionName(), decoration->GetPresetName() };

		if (!manager->IsSyncedPreset(entry.collectionHash, entry.presetHash) || std::find(list.m_Entries.begin(), list.m_Entries.end(), entry) != list.m_Entries.end())
		{
			continue;
		}

		if (list.m_Entries.size() >= kMaxPlayerDecorations)
		{
			break;
		}

		list.m_Entries.push_back(entry);
	}
}

static bool ApplyDecorations(PedDecorationManager* manager, void* ped, const PlayerDecorationList& list)
{
	std::array<PlayerDecorationEntry, kMaxPlayerDecorations> entries;
	uint32_t entryCount = 0;
	bool perfectMatch = true;

	manager->UpdateSyncedPresets();

	for (const auto& entry : list.m_Entries)
	{
		if (entryCount >= entries.size())
		{
			break;
		}

		if (!manager->IsSyncedPreset(entry.collectionHash, entry.presetHash))
		{
			continue;
		}

		entries[entryCount++] = entry;

		uint32_t collectionHash = entry.collectionHash;
		uint32_t presetHash = entry.presetHash;
		perfectMatch = perfectMatch && checkPlaceDecoration(manager, ped, &collectionHash, &presetHash);
	}

	if (perfectMatch && entryCount == g_PedDecorationManager_GetPedDecorationCount(manager, ped))
	{
		return true;
	}

	clearCompressedPedDecorations(manager, ped);

	bool success = true;

	for (uint32_t i = 0; i < entryCount; i++)
	{
		uint32_t collectionHash = entries[i].collectionHash;
		uint32_t presetHash = entries[i].presetHash;
		success = placeDecoration(manager, ped, &collectionHash, &presetHash, list.m_CrewEmblemVariation) && success;
	}

	return success;
}

static bool IsRemotePlayerObject(rage::netObject* netObj)
{
	return netObj && netObj->syncData.isRemote && netObj->objectType == static_cast<uint16_t>(fx::sync::NetObjEntityType::Player);
}

static void CNetObjPlayer_SetPlayerAppearanceData_ClearCompressedPedDecorations(PedDecorationManager* thisPtr, void* ped)
{
	if (g_ApplyingRemoteDecorations)
	{
		return;
	}

	clearCompressedPedDecorations(thisPtr, ped);
}

static uint32_t GetCollectionIndexBits(uint32_t collectionCount)
{
	uint32_t bits = 0;

	while ((1u << bits) < collectionCount)
	{
		bits++;
	}

	return bits;
}

static uint32_t GetSerialisedEntryCount(const PlayerDecorationList& list, std::array<uint32_t, kMaxPlayerDecorations>& collections, uint32_t& collectionCount)
{
	const uint32_t headerBits = 7 + 1 + ((list.m_CrewEmblemVariation != 0) ? 32 : 0) + kDecorationListTrailerBits;
	const uint32_t maxCount = std::min<uint32_t>(uint32_t(list.m_Entries.size()), kMaxPlayerDecorations);

	collectionCount = 0;

	uint32_t count = 0;

	for (; count < maxCount; count++)
	{
		const uint32_t collectionHash = list.m_Entries[count].collectionHash;
		const bool isNewCollection = std::find(collections.begin(), collections.begin() + collectionCount, collectionHash) == collections.begin() + collectionCount;
		const uint32_t newCollectionCount = collectionCount + (isNewCollection ? 1 : 0);
		const uint32_t bits = headerBits + 6 + 32 * newCollectionCount + (count + 1) * (GetCollectionIndexBits(newCollectionCount) + 32);

		if (bits > g_DecorationListMaxBits)
		{
			break;
		}

		if (isNewCollection)
		{
			collections[collectionCount++] = collectionHash;
		}
	}

	return count;
}

//
// count:7 [lastCollectionIndex:6 collectionHash:32*n (collectionIndex:ceil(log2(n)) presetHash:32)*count] hasEmblem:1 [emblem:32] extensionBits:16
//
static void SerialiseDecorationList(SyncDataSerialiser* serialiser, PlayerDecorationList& list, bool isReader)
{
	std::array<uint32_t, kMaxPlayerDecorations> collections;
	uint32_t collectionCount = 0;
	uint32_t entryCount = 0;

	if (!isReader)
	{
		entryCount = GetSerialisedEntryCount(list, collections, collectionCount);
	}

	serialiser->SerialiseUnsigned(entryCount, 7);

	uint32_t extensionBits = 7;

	if (isReader)
	{
		list.m_Entries.clear();
	}

	if (entryCount > 0)
	{
		uint32_t lastCollectionIndex = collectionCount - 1;
		serialiser->SerialiseUnsigned(lastCollectionIndex, 6);

		collectionCount = lastCollectionIndex + 1;

		for (uint32_t i = 0; i < collectionCount; i++)
		{
			serialiser->SerialiseUnsigned(collections[i], 32);
		}

		const uint32_t indexBits = GetCollectionIndexBits(collectionCount);

		extensionBits += 6 + 32 * collectionCount + entryCount * (indexBits + 32);

		for (uint32_t i = 0; i < entryCount; i++)
		{
			uint32_t collectionIndex = 0;
			uint32_t presetHash = 0;

			if (!isReader)
			{
				const auto& entry = list.m_Entries[i];

				collectionIndex = uint32_t(std::find(collections.begin(), collections.begin() + collectionCount, entry.collectionHash) - collections.begin());
				presetHash = entry.presetHash;
			}

			if (indexBits > 0)
			{
				serialiser->SerialiseUnsigned(collectionIndex, indexBits);
			}

			serialiser->SerialiseUnsigned(presetHash, 32);

			if (isReader && collectionIndex < collectionCount && presetHash != 0 && list.m_Entries.size() < kMaxPlayerDecorations)
			{
				list.m_Entries.emplace_back(collections[collectionIndex], presetHash);
			}
		}
	}

	bool hasEmblem = list.m_CrewEmblemVariation != 0;
	serialiser->SerialiseBool(hasEmblem);

	uint32_t emblem = static_cast<uint32_t>(list.m_CrewEmblemVariation);

	if (hasEmblem)
	{
		serialiser->SerialiseUnsigned(emblem, 32);
	}

	extensionBits += 1 + (hasEmblem ? 32 : 0);

	uint32_t trailer = extensionBits;
	serialiser->SerialiseUnsigned(trailer, kDecorationListTrailerBits);

	if (isReader)
	{
		list.m_CrewEmblemVariation = hasEmblem ? static_cast<int32_t>(emblem) : 0;

		// the data is misaligned otherwise
		if (trailer != extensionBits)
		{
			list = {};
		}
	}
}

static void* GetPedFromAppearanceAccessor(void* accessor)
{
	return reinterpret_cast<hook::FlexStruct*>(accessor)->Get<void*>(g_PlayerAppearanceAccessorPedOffset);
}

static void (*g_origExtractPlayerAppearanceData)(void* node, void* accessor);

static void ExtractPlayerAppearanceData(void* node, void* accessor)
{
	g_origExtractPlayerAppearanceData(node, accessor);

	if (!IsDecorationListSyncActive())
	{
		return;
	}

	if (auto ped = GetPedFromAppearanceAccessor(accessor))
	{
		CollectDecorations(ped, g_NodeDecorations);
	}
	else
	{
		g_NodeDecorations = {};
	}
}

static void (*g_origSerialisePlayerAppearanceData)(void* node, SyncDataSerialiser* serialiser);

static void SerialisePlayerAppearanceData(void* node, SyncDataSerialiser* serialiser)
{
	if (serialiser->IsMaximumSizeSerialiser())
	{
		const uint32_t startBits = serialiser->GetSize();
		g_origSerialisePlayerAppearanceData(node, serialiser);

		const uint32_t nodeBits = serialiser->GetSize() - startBits;
		g_DecorationListMaxBits = (nodeBits < kPlayerAppearanceNodeMaxBits) ? std::min(kPlayerAppearanceNodeMaxBits - nodeBits, kDecorationListMaxBits) : 0;

		for (uint32_t bits = 0; bits < g_DecorationListMaxBits; bits += 32)
		{
			uint32_t padding = 0;
			serialiser->SerialiseUnsigned(padding, std::min<int>(32, g_DecorationListMaxBits - bits));
		}

		return;
	}

	if (!IsDecorationListSyncActive())
	{
		g_origSerialisePlayerAppearanceData(node, serialiser);
		return;
	}

	// the list replaces the bitfield, which then isn't written
	const bool isReader = serialiser->IsReader();
	auto& hasDecorations = reinterpret_cast<hook::FlexStruct*>(node)->At<bool>(g_PlayerAppearanceNodeHasDecorationsOffset);
	const bool hadDecorations = hasDecorations;

	if (!isReader)
	{
		hasDecorations = false;
	}

	g_origSerialisePlayerAppearanceData(node, serialiser);

	if (!isReader)
	{
		hasDecorations = hadDecorations;
	}

	SerialiseDecorationList(serialiser, g_NodeDecorations, isReader);
}

static void (*g_origSetPlayerAppearanceData)(void* accessor, void* node);

static void SetPlayerAppearanceData(void* accessor, void* node)
{
	if (!IsDecorationListSyncActive())
	{
		g_origSetPlayerAppearanceData(accessor, node);
		return;
	}

	auto ped = GetPedFromAppearanceAccessor(accessor);
	auto netObj = ped ? GetNetObjectFromEntity(ped) : nullptr;

	if (!IsRemotePlayerObject(netObj))
	{
		g_origSetPlayerAppearanceData(accessor, node);
		return;
	}

	const PlayerDecorationList list = g_NodeDecorations;

	g_ApplyingRemoteDecorations = true;
	g_origSetPlayerAppearanceData(accessor, node);
	g_ApplyingRemoteDecorations = false;

	// the ped is replaced when the model changed
	ped = GetPedFromAppearanceAccessor(accessor);

	if (auto manager = PedDecorationManager::GetInstance(); manager && ped)
	{
		ApplyDecorations(manager, ped, list);
	}
}

// the local player's sync buffers only have room for the vanilla player tree, so they're moved to a larger allocation

static uint32_t g_PlayerSyncDataBufferBits;
static std::unordered_map<void*, std::unique_ptr<uint8_t[]>> g_PlayerSyncDataBuffers;

static void (*datBitBuffer_SetReadWriteBits)(void* buffer, void* data, int maxBits, int baseBitOffset);

static void* (*g_origPlayerSyncDataCtor)(void* syncData);

static void* PlayerSyncDataCtor(void* syncData)
{
	auto result = g_origPlayerSyncDataCtor(syncData);

	const size_t bufferSize = g_PlayerSyncDataBufferBits / 8;
	auto buffers = std::make_unique<uint8_t[]>(bufferSize * 2);

	// rage::netSyncDataBase::GetCurrentStateBuffer/GetShadowBuffer
	auto self = reinterpret_cast<hook::FlexStruct*>(syncData);
	datBitBuffer_SetReadWriteBits(self->CallVirtual<void*>(0x8), &buffers[0], g_PlayerSyncDataBufferBits, 0);
	datBitBuffer_SetReadWriteBits(self->CallVirtual<void*>(0x10), &buffers[bufferSize], g_PlayerSyncDataBufferBits, 0);

	g_PlayerSyncDataBuffers[syncData] = std::move(buffers);

	return result;
}

static void* (*g_origPlayerSyncDataDtor)(void* syncData, uint32_t flags);

static void* PlayerSyncDataDtor(void* syncData, uint32_t flags)
{
	auto result = g_origPlayerSyncDataDtor(syncData, flags);
	g_PlayerSyncDataBuffers.erase(syncData);

	return result;
}

static void PatchPlayerSyncBuffers()
{
	// CPlayerSyncData ctor: SetReadWriteBits(&m_CurrentStateBuffer, m_CSBuffer, bufferSizeInBytes << 3, 0)
	auto location = hook::get_pattern<char>("48 8D 96 00 0D 00 00 45 33 C9 41 B8 ? ? 00 00 48 8B CB E8 ? ? ? ? 48 8D 96 ? ? 00 00 45 33 C9 41 B8");
	const uint32_t originalBits = *reinterpret_cast<uint32_t*>(location + 12);
	const uint32_t extraBytes = (kDecorationListMaxBits + 7) / 8 + 1;

	g_PlayerSyncDataBufferBits = originalBits + extraBytes * 8;
	hook::set_call(&datBitBuffer_SetReadWriteBits, location + 19);

	char* createSyncData = nullptr;
	char* createSyncTree = nullptr;
	int createSyncTreeCount = 0;

	// CNetObjPlayer::CreateSyncData
	{
		auto matches = hook::pattern("48 85 C0 74 14 48 8B C8 E8 ? ? ? ? 48 8D 05 ? ? ? ? 48 89 03 EB 02 33 DB").count_hint(46);

		for (size_t i = 0; i < matches.size(); i++)
		{
			auto match = matches.get(i).get<char>(0);
			auto ctor = hook::get_call(match + 8);

			if (location >= ctor && location < ctor + 0x100)
			{
				createSyncData = match;
				break;
			}
		}
	}

	{
		auto matches = hook::pattern("48 85 C0 74 ? BA ? ? 00 00 48 8B C8 E8 ? ? ? ? 48 89 05").count_hint(12);

		for (size_t i = 0; i < matches.size(); i++)
		{
			auto match = matches.get(i).get<char>(0);

			if (*reinterpret_cast<uint32_t*>(match + 6) == originalBits / 8)
			{
				createSyncTree = match;
				createSyncTreeCount++;
			}
		}
	}

	if (!createSyncData || createSyncTreeCount != 1)
	{
		FatalError("Failed to locate the player sync data buffers. Please report this at https://github.com/citizenfx/fivem.");
	}

	hook::put<uint32_t>(createSyncTree + 6, g_PlayerSyncDataBufferBits / 8);

	hook::set_call(&g_origPlayerSyncDataCtor, createSyncData + 8);
	hook::call(createSyncData + 8, PlayerSyncDataCtor);

	auto vtable = hook::get_address<uintptr_t*>(createSyncData + 0x10);
	g_origPlayerSyncDataDtor = reinterpret_cast<decltype(g_origPlayerSyncDataDtor)>(vtable[0]);
	hook::put<uintptr_t>(&vtable[0], reinterpret_cast<uintptr_t>(PlayerSyncDataDtor));
}

static HookFunction hookFunction([]
{
	hook::set_call(&clearCompressedPedDecorations, hook::get_pattern("49 8B D7 49 8B CD E8 ? ? ? ? 85 F6 74", 6));
	hook::set_call(&checkPlaceDecoration, hook::get_pattern("49 8B D7 49 8B CD E8 ? ? ? ? 84 C0 74 ? 41 B0 01", 6));
	hook::set_call(&placeDecoration, hook::get_pattern("44 89 74 24 20 89 44 24 38 E8", 9));

	{
		auto location = hook::get_pattern<char>("48 8B 05 ? ? ? ? 4C 8B 88 ? ? ? ? 8B 01");

		PedDecorationManager::ms_Instance = hook::get_address<PedDecorationManager**>(location + 3);
		PedDecorationManager::kCollectionsOffset = *reinterpret_cast<int32_t*>(location + 10);
	}

	{
		auto location = hook::get_pattern<char>("48 85 D2 74 ? 8A 82 ? ? ? ? 4C 8B 05 ? ? ? ? 3C FF 74 ? 0F B6 C0 49 8B 8C C0 ? ? ? ? 48 8B 01 EB ? 33 C0 48 85 C0 74 ? 0F B7 80 ? ? ? ? C3 8A 82");

		CPedDamageManager::ms_Instance = hook::get_address<CPedDamageManager**>(location + 0xE);
		CPedDamageManager::kPedDamageSetIdOffset = *reinterpret_cast<int32_t*>(location + 0x37);
		CPedDamageManager::kDamageSetPoolOffset = *reinterpret_cast<int32_t*>(location + 0x46);
		CPedDamageSet::kDecorationCountOffset = *reinterpret_cast<int32_t*>(location + 0x56);
		g_PedDecorationManager_GetPedDecorationCount = reinterpret_cast<decltype(g_PedDecorationManager_GetPedDecorationCount)>(location);
	}

	hook::call(hook::get_pattern("44 38 AF ? ? ? ? 75 ? E8 ? ? ? ? 85 C0 74 ? 48 8B D6 48 8B CB E8 ? ? ? ? EB ? 8B 87 ? ? ? ? 4C 8D 87 ? ? ? ? 41 B9 ? ? ? ? 89 44 24 20 E8", 0x18), CNetObjPlayer_SetPlayerAppearanceData_ClearCompressedPedDecorations);

	// CPlayerAppearanceDataNode
	g_PlayerAppearanceNodeHasDecorationsOffset = *hook::get_pattern<int32_t>("88 ? ? ? ? ? 84 C0 75 0D 44 8B C5 33", 2);

	{
		auto location = hook::get_pattern<char>("48 8B B1 ? ? ? ? 45 33 ED 48 8B FA 4C 8B F1 48 85 F6", -0x1C);

		g_PlayerAppearanceAccessorPedOffset = *reinterpret_cast<int32_t*>(location + 0x1F);
		g_origSetPlayerAppearanceData = hook::trampoline(location, SetPlayerAppearanceData);
	}

	g_origExtractPlayerAppearanceData = hook::trampoline(hook::get_pattern("48 8B 02 4C 8B C2 48 8B D9 48 8B D1 49 8B C8 FF 50 ? 66 83 BB ? 04 00 00 00 0F 95 C0 88 83", -6), ExtractPlayerAppearanceData);
	// 5 bytes are already used/replaced by CrashFixes.PedHeadBlendData
	g_origSerialisePlayerAppearanceData = hook::trampoline(hook::get_pattern("41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 38 48 8B FA 48 8D 91 ? ? 00 00 48 8B F1 48 8B CF 45 33 C0 E8", -5), SerialisePlayerAppearanceData);

	PatchPlayerSyncBuffers();

	OnKillNetworkDone.Connect([]
	{
		g_NodeDecorations = {};
	});
});
