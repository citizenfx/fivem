#include <StdInc.h>
#include <EntitySystem.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <atArray.h>
#include <Streaming.h>
#include <ICoreGameInit.h>
#include <Error.h>

#include <unordered_set>

static hook::cdecl_stub<fwEntity* (int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

fwEntity* rage::fwScriptGuid::GetBaseFromGuid(int handle)
{
	return getScriptEntity(handle);
}

int rage::fwScriptGuid::GetGuidFromBase(rage::fwEntity* base)
{
	return int(getScriptGuidForEntity(base));
}

static hook::cdecl_stub<void* (fwExtensionList*, uint32_t)> getExtension([]()
{
	return hook::get_pattern("8B FA 83 FA 20 73 17 8B C7", -15);
});

static hook::cdecl_stub<void(fwExtensionList*, rage::fwExtension*)> addExtension([]()
{
	return hook::get_pattern("48 8B 03 FF 50 18 83 F8 20 73 21", -0x2A);
});

void fwExtensionList::Add(rage::fwExtension* extension)
{
	return addExtension(this, extension);
}

void* fwExtensionList::Get(uint32_t id)
{
	return getExtension(this, id);
}

static uint32_t* fwSceneUpdateExtension_classId;

uint32_t fwSceneUpdateExtension::GetClassId()
{
	return *fwSceneUpdateExtension_classId;
}

static PIMAGE_SECTION_HEADER GetSection(std::string_view name, int off = 0)
{
	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)GetModuleHandle(NULL);
	PIMAGE_NT_HEADERS ntHeader = (PIMAGE_NT_HEADERS)((char*)dosHeader + dosHeader->e_lfanew);
	IMAGE_SECTION_HEADER* section = IMAGE_FIRST_SECTION(ntHeader);

	int matchIdx = -1;

	for (int i = 0; i < ntHeader->FileHeader.NumberOfSections; i++)
	{
		if (name == (char*)section->Name)
		{
			matchIdx++;

			if (off == matchIdx)
			{
				return section;
			}
		}

		section++;
	}

	return NULL;
}

struct NewTypeId
{
	NewTypeId* Self;
	uint32_t Hash;
};

static std::unordered_map<uint32_t, void*> s_newTypesIds;

static void FindNewTypeIds()
{
	PIMAGE_SECTION_HEADER section = GetSection(".data");

	auto here = (uintptr_t)GetModuleHandle(NULL) + section->VirtualAddress;
	auto end = here + section->Misc.VirtualSize - sizeof(NewTypeId);

	for (; here <= end; here += sizeof(void*))
	{
		auto type = (NewTypeId*)here;

		if (type->Self != type)
			continue;

		// This'll probably find some false positives, but that shouldn't matter

		s_newTypesIds.emplace(type->Hash, type);
	}
}

static hook::thiscall_stub<void(fwEntity* self)> fwEntity__ProtectStreamedArchetype([]
{
	return hook::get_pattern("40 53 48 83 EC ? 48 8B D9 48 8B 49 ? 8B 41 ? C1 E8");
});

void fwEntity::ProtectStreamedArchetype()
{
	fwEntity__ProtectStreamedArchetype(this);
}

bool fwEntity::IsOfTypeH(uint32_t hash)
{
	void** vtbl = *(void***)this;

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		void* ptr = s_newTypesIds.at(hash);
		return ((bool(*)(fwEntity*, void*)) vtbl[5])(this, ptr);
	}

	if (xbr::IsGameBuildOrGreater<2189>())
	{
		return ((bool(*)(fwEntity*, const uint32_t&)) vtbl[1])(this, hash);
	}

	return ((bool(*)(fwEntity*, uint32_t)) vtbl[1])(this, hash);
}

namespace rage
{
struct atBitSet
{
	uint32_t* m_Bits;
	uint16_t m_Size;
	uint16_t m_BitSize;

	bool Test(int index)
	{
		return (m_Bits[(index >> 5)] >> (index & 0x1F)) & 1;
	}

	void Clear(int index)
	{
		m_Bits[(index >> 5)] &= ~(1u << (index & 0x1F));
	}
};

struct atPoolBase
{
	char* m_Pool;
	size_t m_Size;
	size_t m_FreeCount;
	size_t m_ElemSize;
	void* m_FirstFree;
	bool m_OwnMem;

	void* At(int index)
	{
		return &m_Pool[index * m_ElemSize];
	}

	void Delete(void* ptr)
	{
		if (ptr)
		{
			*(void**)ptr = m_FirstFree;
			m_FirstFree = ptr;
			++m_FreeCount;
		}
	}
};

template<typename T>
struct atIteratablePool : atPoolBase
{
	atBitSet m_AllocatedMap;

	T* TryGetAt(int index)
	{
		return ((index < m_Size) && m_AllocatedMap.Test(index)) ? (T*)At(index) : nullptr;
	}

	bool DeleteAt(int index)
	{
		if (m_AllocatedMap.Test(index))
		{
			m_AllocatedMap.Clear(index);

			atPoolBase::Delete(At(index));

			return true;
		}

		return false;
	}
};

template<typename Key, typename Value>
struct atMapMemoryPool
{
	atMapMemoryPool* mp_Pool;
	int m_iUsed;
	void* m_Head;
	void* m_PoolBase;
};

template <typename Key, typename Value>
struct atSimplePooledMapType
{
	struct Entry
	{
		Key key;
		Value value;
		Entry* next;
	};

	atMapMemoryPool<Key, Value> m_Pool;

	struct
	{
		Entry** m_Hash;
		uint16_t m_Slots;
		uint16_t m_Used;
		char m_HashFn;
		char m_Equals;
		atMapMemoryPool<Key, Value>* m_Memory;
		bool m_AllowReCompute;
	} m_Map;

	int m_Size;
};
}

static rage::atIteratablePool<fwArchetype*>* fwArchetypeManager__ms_ArchetypePool;
static rage::atSimplePooledMapType<uint32_t, rage::fwModelId>* fwArchetypeManager__ms_ArchetypeMap;
static uint32_t* fwArchetypeManager__ms_streamingId;

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, rage::fwModelId& id)> getArchetype([]()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

fwArchetype* rage::fwArchetypeManager::GetArchetypeFromHashKey(uint32_t hash, fwModelId& id)
{
	return getArchetype(hash, id);
}

// Just do a bunch of sanity checks to make sure our archetype pointer is still valid
static bool ValidateArchetype(fwArchetype* archetype)
{
	if (!archetype)
	{
		return false;
	}

	if (archetype->assignedStreamingSlot == 0xFFFF)
	{
		return false;
	}

	if (archetype->assignedStreamingSlot != archetype->cachedModelId.modelIndex)
	{
		return false;
	}

	if (archetype->hash != archetype->hashCopy)
	{
		return false;
	}

	auto ppArchetype = fwArchetypeManager__ms_ArchetypePool->TryGetAt(archetype->assignedStreamingSlot);

	if (!ppArchetype)
	{
		return false;
	}

	if (*ppArchetype != archetype)
	{
		return false;
	}

	return true;
}

static const rage::fwModelId InvalidModelId;

const rage::fwModelId& rage::fwArchetypeManager::LookupModelId(fwArchetype* archetype)
{
	if (!ValidateArchetype(archetype))
	{
		// In a perfect world this would never fail.
		// Unfortunately, the order of unloading custom assets is very confusing/broken,
		// and sometimes archetypes are deleted before all the entities using them are destroyed.
		// There are so many layers of hacks and workarounds which aren't worth investigating for
		// now, so just accept that we will be given a few invalid archetypes.

		trace("Looking up invalid archetype pointer (maybe %08X or %08X)\n", archetype->hash, archetype->hashCopy);

		return InvalidModelId;
	}

	return archetype->cachedModelId;
}

void rage::fwArchetypeManager::UnregisterStreamedArchetype(fwArchetype* archetype)
{
	// We are only registered into the archetype pool/map if we have a valid streaming slot,
	// and only unregistered (during destruction) if the slot is valid.
	// Unlike our hash, our streaming slot is unique for the lifetime of our archetype.
	assert(ValidateArchetype(archetype));
	
	fwArchetypeManager__ms_ArchetypePool->DeleteAt(archetype->assignedStreamingSlot);

	auto& pMap = fwArchetypeManager__ms_ArchetypeMap->m_Map;
	bool found = false;

	// There might be multiple map entries corresponding to our hash, so make sure it also matches our streaming slot
	for (auto ppEntry = &pMap.m_Hash[archetype->hash % pMap.m_Slots]; *ppEntry; ppEntry = &(*ppEntry)->next)
	{
		auto pEntry = *ppEntry;

		if (pEntry->key != archetype->hash)
		{
			continue;
		}

		if (pEntry->value.modelIndex != archetype->assignedStreamingSlot)
		{
			continue;
		}

		*ppEntry = pEntry->next;

		auto pPool = pMap.m_Memory;

		if (pPool)
		{
			*(void**)pEntry = pPool->m_Head;
			pPool->m_Head = pEntry;
			--pPool->m_iUsed;
		}

		--pMap.m_Used;

		found = true;
		break;
	}

	assert(found);

	uint32_t baseIndex = GetStreamingModule()->baseIdx;

	streaming::Manager::GetInstance()->UnregisterObject(baseIndex + archetype->assignedStreamingSlot);
}

static std::string GetMapTypeDefName(uint16_t index)
{
	switch (index)
	{
		case 0x0000:
		case 0xF000:
			return "Extra";

		case 0x0FFF:
		case 0xFFFF:
			return "Global";

		default:
		{
			std::string result;

			if (index < 0xFFF)
			{
				uint32_t baseIndex = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ytyp")->baseIdx;
				result = streaming::GetStreamingNameForIndex(baseIndex + index);
			}

			return !result.empty() ? result : va("%03X", index);
		}
	}
}

static std::unordered_set<uint32_t> SeenDuplicates;

static void CheckForDuplicateArchetypes(uint32_t hash, uint16_t mapTypeDefIndex)
{
	if (SeenDuplicates.find(hash) != SeenDuplicates.end())
	{
		return;
	}

	rage::fwModelId id;

	if (!rage::fwArchetypeManager::GetArchetypeFromHashKey(hash, id))
	{
		return;
	}

	SeenDuplicates.insert(hash);

	std::string modelName = streaming::GetStreamingBaseNameForHash(hash);
	std::string mapName1 = GetMapTypeDefName(mapTypeDefIndex);
	std::string mapName2 = GetMapTypeDefName(id.mapTypesIndex);

	trace("Duplicate Archetype '%s' (%08X), seen in '%s' and '%s'\n", modelName, hash, mapName1, mapName2);
}

static bool TooManyArchetypes = false;

static void SetArchetypeModelId(fwArchetype* archetype, uint32_t mapTypeDefIndex)
{
	if ((archetype->assignedStreamingSlot == 0xFFFF) && !TooManyArchetypes)
	{
		TooManyArchetypes = true;

		// Failure to register our archetype shouldn't be fatal, but will break loading our model.
		trace("Archetype registration failed (FreeCount=%i)\n", fwArchetypeManager__ms_ArchetypePool->m_FreeCount);
	}

	// Store a second copy of the hash for debugging/validation purposes
	archetype->hashCopy = archetype->hash;

	// Store our own copy of the ModelId, instead of having to retrieve it from the ArchetypeMap.
	// This is faster, and avoids mismatches with duplicate archetypes
	archetype->cachedModelId = {};
	archetype->cachedModelId.modelIndex = archetype->assignedStreamingSlot;
	archetype->cachedModelId.mapTypesIndex = mapTypeDefIndex & 0xFFF;
	archetype->cachedModelId.isStreamed = archetype->streaming;
}

static uint16_t (*Orig_RegisterPermanentArchetype)(fwArchetype* archetype, uint32_t mapTypeDefIndex, bool bMemLock);
uint16_t rage::fwArchetypeManager::RegisterPermanentArchetype(fwArchetype* archetype, uint32_t mapTypeDefIndex, bool bMemLock)
{
	CheckForDuplicateArchetypes(archetype->hash, mapTypeDefIndex);
	uint16_t result = Orig_RegisterPermanentArchetype(archetype, mapTypeDefIndex, bMemLock);
	SetArchetypeModelId(archetype, mapTypeDefIndex);
	return result;
}

static uint16_t (*Orig_RegisterStreamedArchetype)(fwArchetype* archetype, uint32_t mapTypeDefIndex);
uint16_t rage::fwArchetypeManager::RegisterStreamedArchetype(fwArchetype* archetype, uint32_t mapTypeDefIndex)
{
	CheckForDuplicateArchetypes(archetype->hash, mapTypeDefIndex);
	uint16_t result = Orig_RegisterStreamedArchetype(archetype, mapTypeDefIndex);
	SetArchetypeModelId(archetype, mapTypeDefIndex);
	return result;
}

streaming::strStreamingModule* rage::fwArchetypeManager::GetStreamingModule()
{
	return streaming::Manager::GetInstance()->moduleMgr.modules[*fwArchetypeManager__ms_streamingId];
}

static HookFunction hookFunction([] {
	fwSceneUpdateExtension_classId = hook::get_address<uint32_t*>(hook::get_pattern("48 8B D3 48 89 73 08 E8", -64));

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		FindNewTypeIds();
	}

	hook::jump(hook::get_pattern("0F B7 05 ? ? ? ? 44 8B 49 ? 45 33 C0"), &rage::fwArchetypeManager::LookupModelId);
	hook::jump(hook::get_call(hook::get_pattern("E8 ? ? ? ? 80 7B ? ? 74 ? 48 8B 4B ? 48 85 C9")), &rage::fwArchetypeManager::UnregisterStreamedArchetype);

	{
		auto ptr = hook::get_pattern<char>("E8 ? ? ? ? EB ? 45 33 C0 48 8B CB E8");
		Orig_RegisterStreamedArchetype = hook::trampoline(hook::get_call(ptr), &rage::fwArchetypeManager::RegisterStreamedArchetype);
		Orig_RegisterPermanentArchetype = hook::trampoline(hook::get_call(ptr + 13), &rage::fwArchetypeManager::RegisterPermanentArchetype);
	}

	fwArchetypeManager__ms_ArchetypePool = hook::get_address<decltype(fwArchetypeManager__ms_ArchetypePool)>(hook::get_pattern("48 8D 0D ? ? ? ? 45 33 C0 E8 ? ? ? ? 0F B7 1D", 3));
	fwArchetypeManager__ms_ArchetypeMap = hook::get_address<decltype(fwArchetypeManager__ms_ArchetypeMap)>(hook::get_pattern("48 8D 0D ? ? ? ? 8B D3 89 1D ? ? ? ? E8 ? ? ? ? 48 8D 0D", 3));
	fwArchetypeManager__ms_streamingId = hook::get_address<decltype(fwArchetypeManager__ms_streamingId)>(hook::get_pattern("44 8B 05 ? ? ? ? 4E 8B 04 C2", 3));

	Instance<ICoreGameInit>::Get()->OnShutdownSession.Connect([]()
	{
		TooManyArchetypes = false;

		SeenDuplicates.clear();
	});
});
