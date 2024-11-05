#include <StdInc.h>
#include <EntitySystem.h>

#include <Hooking.h>

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

static HookFunction hookFunction([]
{
	fwSceneUpdateExtension_classId = hook::get_address<uint32_t*>(hook::get_pattern("48 8B D3 48 89 73 08 E8", -64));

	if (xbr::IsGameBuildOrGreater<2802>())
	{
		FindNewTypeIds();
	}
});

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, rage::fwModelId& id)> getArchetype([]()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

fwArchetype* rage::fwArchetypeManager::GetArchetypeFromHashKey(uint32_t hash, fwModelId& id)
{
	return getArchetype(hash, id);
}

fwArchetype* rage::fwArchetypeManager::GetArchetypeFromHashKeySafe(uint32_t hash, fwModelId& id)
{
	__try
	{
		return getArchetype(hash, id);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return nullptr;
	}
}
