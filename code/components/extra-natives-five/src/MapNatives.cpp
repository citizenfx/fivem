/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>

#ifdef GTA_FIVE

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <ScriptEngine.h>
#include <filesystem>
#include <Resource.h>
#include <ResourceCache.h>
#include <ResourceMetaDataComponent.h>
#include <ResourceEventComponent.h>
#include <ResourceManager.h>
#include <NetLibrary.h>
#include <VFSManager.h>
#include <ICoreGameInit.h>
#include <tinyxml2.h>

struct strStreamingInterface
{
	virtual ~strStreamingInterface() = 0;

	virtual void LoadAllRequestedObjects(bool) = 0;

	virtual void RequestFlush() = 0;
};

DLL_IMPORT NetLibrary* g_netLibrary;
DLL_IMPORT fwEvent<const char*> OnLoadContentXML;
DLL_IMPORT uint32_t GetCurrentMapGroup();


static bool(__fastcall* LoadDLCRpf)(void* pThis, const char* path) = nullptr;
static void(__fastcall* ShutdownSession)(uint32_t mode) = nullptr;

static void(__fastcall* RevertContentChangeSetGroup)(void* pThis, uint32_t changeSet) = nullptr;
static void(__fastcall* ExecuteContentChangeSetGroup)(void* pThis, uint32_t changeSet) = nullptr;
static void(__fastcall* ExecuteContentChangeSet)(void* pThis, uint32_t* group, uint32_t* name) = nullptr;
static void(__fastcall* RevertContentChangeSet)(void* pThis, uint32_t* group, uint32_t* name, uint32_t flags) = nullptr;

static std::vector<std::tuple<uint32_t*, uint32_t*>> s_MapConds;
static uint32_t s_OverrideMap = 0xFFFFFFFF;
static void** s_ExtraContentManager = nullptr;
static strStreamingInterface** s_strStreamingInterface = nullptr;

static const uint32_t GROUP_MAP_SP = HashString("GROUP_MAP_SP");
static const uint32_t GROUP_MAP = HashString("GROUP_MAP");
static const uint32_t GROUP_MAP_EMPTY = HashString("GROUP_MAP_EMPTY");

static std::unordered_map<uint32_t, std::string> s_ContentChangesetGroupHashes;
static std::unordered_map<uint32_t, std::string> s_ContentChangesetHashes;

static std::string& ResolveHash(const std::unordered_map<uint32_t, std::string>& map, uint32_t hash, std::string& out)
{
	auto it = map.find(hash);

	if (it == map.end())
	{
		out = va("0x%X", hash);
	}
	else
	{
		out= it->second.c_str();
	}

	return out;
}

static void __fastcall hk_ExecuteContentChangeSet(void* pThis, uint32_t* group, uint32_t* name)
{
	{
		static bool emptyMapMounted = false;

		if (!emptyMapMounted)
		{
			emptyMapMounted = true;

			if (LoadDLCRpf(*s_ExtraContentManager, "emptymap/"))
			{
				trace("mounted emptymap ccs\n");
			}
			else
			{
				trace("Error mounting emptymap ccs\n");
			}
		}
	}

	bool isMap = *group == s_OverrideMap;
	uint32_t curr = GetCurrentMapGroup();

	std::string groupName;
	std::string changesetName;

	ResolveHash(s_ContentChangesetGroupHashes, *group, groupName);
	ResolveHash(s_ContentChangesetHashes, *name, changesetName);

	trace("ExecuteContentChangeSet %s %s%s\n", groupName.c_str(), changesetName.c_str(), isMap ? " [map]" : "");

	if (isMap)
	{
		for (auto& [mpc, spc] : s_MapConds)
		{
			*spc = *group;
			*mpc = curr;
		}
	}

	ExecuteContentChangeSet(pThis, group, name);

	if (isMap)
	{
		for (auto& [mpc, spc] : s_MapConds)
		{
			*spc = GROUP_MAP_SP;
			*mpc = GROUP_MAP;
		}
	}

	if (s_OverrideMap != 0xFFFFFFFF)
	{
		g_netLibrary->RunFrame(); // custom maps can block for too long
	}
}

static void __fastcall hk_RevertContentChangeSet(void* pThis, uint32_t* group, uint32_t* name, uint32_t flags)
{
	bool isMap = *group == s_OverrideMap;
	uint32_t curr = GetCurrentMapGroup();

	std::string groupName;
	std::string changesetName;

	ResolveHash(s_ContentChangesetGroupHashes, *group, groupName);
	ResolveHash(s_ContentChangesetHashes, *name, changesetName);

	trace("RevertContentChangeSet %s %s%s\n", groupName.c_str(), changesetName.c_str(), isMap ? " [map]" : "");

	if (isMap)
	{
		for (auto& [mpc, spc] : s_MapConds)
		{
			*spc = *group;
			*mpc = curr;
		}
	}

	RevertContentChangeSet(pThis, group, name, flags);

	if (isMap)
	{
		for (auto& [mpc, spc] : s_MapConds)
		{
			*spc = GROUP_MAP_SP;
			*mpc = GROUP_MAP;
		}
	}

	if (s_OverrideMap != 0xFFFFFFFF)
	{
		g_netLibrary->RunFrame(); // custom maps can block for too long
	}
}

static HookFunction hookFunction([]()
{
	ShutdownSession = (decltype(ShutdownSession))hook::get_pattern("40 53 48 83 EC 20 33 DB 83 F9 01");

	uint8_t* ptr = (uint8_t*)hook::get_pattern("41 81 3F 79 91 C8 BC");
	s_MapConds.emplace_back((uint32_t*)(ptr + 3), (uint32_t*)(ptr + 12));

	ptr = (uint8_t*)hook::get_pattern("81 FB 79 91 C8 BC");
	s_MapConds.emplace_back((uint32_t*)(ptr + 2), (uint32_t*)(ptr + 10));

	ptr = (uint8_t*)hook::get_pattern("81 3C F9 79 91 C8 BC");
	s_MapConds.emplace_back((uint32_t*)(ptr + 3), (uint32_t*)(ptr + 12));

	DWORD op;
	for (auto& [mpc, spc] : s_MapConds)
	{
		VirtualProtect(mpc, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &op);
		VirtualProtect(spc, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &op);
	}

	LoadDLCRpf = (decltype(LoadDLCRpf))hook::get_pattern("48 8B C4 48 89 58 ? 48 89 70 ? 48 89 78 ? 55 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 8B F9 48 8B DA");
	ExecuteContentChangeSet = hook::trampoline(hook::get_pattern("48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 8D AC 24 B0"), hk_ExecuteContentChangeSet);
	RevertContentChangeSet = hook::trampoline(hook::get_pattern("48 89 5C 24 18 55 56 57 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 70 0F"), hk_RevertContentChangeSet);
	
	ptr = (uint8_t*)hook::get_pattern("C6 05 ? ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? BA E2 99 8F 57");
	//s_MapDataStore = reinterpret_cast<void**>(ptr + *reinterpret_cast<int*>(ptr - 4));

	ptr += 7;
	//s_FlagLoadingSp = reinterpret_cast<bool*>(ptr + *reinterpret_cast<int*>(ptr - 5));

	ptr += 5 + 7;
	s_ExtraContentManager = (decltype(s_ExtraContentManager))(ptr + *(int*)(ptr - 4));

	ptr += 10;
	RevertContentChangeSetGroup = (decltype(RevertContentChangeSetGroup))(ptr + *(int*)(ptr - 4));

	ptr += 17;
	ExecuteContentChangeSetGroup = (decltype(ExecuteContentChangeSetGroup))(ptr + *(int*)(ptr - 4));

	ptr = (uint8_t*)hook::get_pattern("48 8B 0D ? ? ? ? 48 8B 01 FF 90 90 00 00 00 B9", 7);
	s_strStreamingInterface = (decltype(s_strStreamingInterface))(ptr + *(int*)(ptr - 4));
});

static void UnloadMap()
{
	RevertContentChangeSetGroup(*s_ExtraContentManager, GetCurrentMapGroup());

	s_OverrideMap = GROUP_MAP_EMPTY;
	ExecuteContentChangeSetGroup(*s_ExtraContentManager, GROUP_MAP_EMPTY);
	s_OverrideMap = 0xFFFFFFFF;

	ShutdownSession(0);

	(*s_strStreamingInterface)->RequestFlush();
}

static void ReloadMap()
{
	s_OverrideMap = GROUP_MAP_EMPTY;
	RevertContentChangeSetGroup(*s_ExtraContentManager, GROUP_MAP_EMPTY);
	s_OverrideMap = 0xFFFFFFFF;

	ExecuteContentChangeSetGroup(*s_ExtraContentManager, GetCurrentMapGroup());

	ShutdownSession(0);

	(*s_strStreamingInterface)->RequestFlush();
}

static void CollectContentXMLHashes(const char* name)
{
	std::filesystem::path p = name;
	auto setup2xmlPath = p.parent_path() / "setup2.xml";
	auto setup2xml = vfs::OpenRead(setup2xmlPath.string());
	auto buff = setup2xml->ReadToEnd();
	auto xml = std::string(buff.begin(), buff.end());

	tinyxml2::XMLDocument doc;

	if (doc.Parse(reinterpret_cast<const char*>(xml.data()), xml.size()) == tinyxml2::XML_SUCCESS)
	{
		tinyxml2::XMLElement* root = doc.RootElement();
		if (!root)
			return;

		tinyxml2::XMLElement* groups = root->FirstChildElement("contentChangeSetGroups");
		if (!groups)
			return;

		for (tinyxml2::XMLElement* item = groups->FirstChildElement("Item"); item != nullptr; item = item->NextSiblingElement("Item"))
		{
			tinyxml2::XMLElement* nameHashElem = item->FirstChildElement("NameHash");
			if (nameHashElem && nameHashElem->GetText())
			{
				const char* groupName = nameHashElem->GetText();
				uint32_t groupHash = HashString(groupName);
				s_ContentChangesetGroupHashes[groupHash] = groupName;

				tinyxml2::XMLElement* changeSets = item->FirstChildElement("ContentChangeSets");
				if (changeSets)
				{
					for (tinyxml2::XMLElement* csItem = changeSets->FirstChildElement("Item"); csItem != nullptr; csItem = csItem->NextSiblingElement("Item"))
					{
						if (csItem->GetText())
						{
							const char* changeSetName = csItem->GetText();
							uint32_t changeSetHash = HashString(changeSetName);
							s_ContentChangesetHashes[changeSetHash] = changeSetName;
						}
					}
				}
			}
		}
	}
}

static InitFunction initFunction([]()
{
	fx::ScriptEngine::RegisterNativeHandler("UNLOAD_MAP", [](fx::ScriptContext& context)
	{
		UnloadMap();
	});

	fx::ScriptEngine::RegisterNativeHandler("RELOAD_MAP", [](fx::ScriptContext& context)
	{
		ReloadMap();
	});

	OnLoadContentXML.Connect(CollectContentXMLHashes);
});

#endif
