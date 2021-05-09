#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>

#include <CoreConsole.h>
#include <GamePrimitives.h>

#include <MinHook.h>

#include <ResourceManager.h>
#include <ResourceEventComponent.h>

#include <ScriptEngine.h>
#include <scrBind.h>

#include <GameInit.h>
#include <Streaming.h>

namespace rage
{
struct strLocalIndex
{
	uint32_t id;

	explicit strLocalIndex(uint32_t id)
		: id(id)
	{
	}
};
}

static hook::thiscall_stub<rage::strLocalIndex(streaming::strStreamingModule*, const uint32_t& hash)> _assetStore_getIndexByKey([]()
{
	return hook::get_pattern("4C 8B D1 F7 B1 80 00 00 00 48 8B 41 78", -0x13);
});

static rage::fwEntity* (*g_origConstructEntity)(fwEntityDef* entityDef, int mapDataIdx, rage::fwArchetype* archetype, void* unkId);
static int g_curEntityIndex;

static auto IsMapDataCustom(int localIdx)
{
	static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
	auto strIdx = mapDataStore->baseIdx + localIdx;

	const auto& strEntry = streaming::Manager::GetInstance()->Entries[strIdx];
	return (strEntry.handle & 0xFFFF) < 2;
}

class MapDataOwnerExtension : public rage::fwExtension
{
public:
	MapDataOwnerExtension(fwEntityDef* entityDef, int mapDataIdx, int entityIdx)
	{
		this->guid = entityDef->guid;
		this->mapDataIdx = mapDataIdx;
		this->entityIdx = entityIdx;

		static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
		auto strIdx = mapDataStore->baseIdx + mapDataIdx;

		const auto& strEntry = streaming::Manager::GetInstance()->Entries[strIdx];

		// #TODO: clean up the +56 everywhere
		auto pool = (atPoolBase*)((char*)mapDataStore + 56);
		if (auto entry = pool->GetAt<char>(mapDataIdx))
		{
			auto mapDataHash = *(uint32_t*)(entry + 12);
			this->mapDataHash = mapDataHash;
		}

		// custom entries are either in pgRawStreamer 0 or our custom variant at 1
		if (IsMapDataCustom(mapDataIdx))
		{
			this->isCustom = true;
		}
	}

	virtual ~MapDataOwnerExtension() = default;

	virtual int GetExtensionId() const override
	{
		return GetClassId();
	}

	static int GetClassId()
	{
		return 64;
	}

	std::string Format() const
	{
		static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
		auto strIdx = mapDataStore->baseIdx + mapDataIdx;

		return fmt::sprintf("MapDataOwnerExtension(guid %08x -> %d (%s) @ %d)", guid, mapDataIdx, streaming::GetStreamingNameForIndex(strIdx), entityIdx);
	}

	uint32_t GetMapData() const
	{
		return mapDataHash;
	}

	uint32_t GetInternalIndex() const
	{
		return isCustom ? entityIdx : guid;
	}

	uint32_t GetGuid() const
	{
		return guid;
	}

private:
	uint32_t guid = 0;
	uint32_t mapDataHash = 0;
	int mapDataIdx = 0;
	int entityIdx = 0;
	bool isCustom = false;
};

class CustomMatrixDef : public fwExtensionDef
{
public:
	CustomMatrixDef()
	{
		name = HashRageString("CustomMatrixDef");
	}

	CustomMatrixDef(const float* matrix)
		: CustomMatrixDef()
	{
		mat44 = *(const Matrix4x4*)matrix;
	}

	virtual ~CustomMatrixDef() override = default;

	virtual void* parser_GetStructure() override
	{
		return nullptr;
	}

	Matrix4x4 mat44;
};

static bool g_isEditorRuntime;

static rage::fwEntity* ConstructEntity(fwEntityDef* entityDef, int mapDataIdx, rage::fwArchetype* archetype, void* unkId)
{
	auto entity = g_origConstructEntity(entityDef, mapDataIdx, archetype, unkId);

	if (entity)
	{
		if (g_isEditorRuntime)
		{
			entity->AddExtension(new MapDataOwnerExtension(entityDef, mapDataIdx, g_curEntityIndex));
		}

		// check for a custom matrix override that may have been persisted
		for (auto& extensionDef : entityDef->extensions)
		{
			if (extensionDef->name == HashRageString("CustomMatrixDef"))
			{
				CustomMatrixDef* ext = static_cast<CustomMatrixDef*>(extensionDef);
				entity->UpdateTransform(ext->mat44, true);
			}
		}
	}

	return entity;
}

static void (*g_origMapDataContents_Create)(CMapDataContents* contents, void* a2, CMapData* mapData, uint32_t mapDataIdx);

static void MapDataContents_Create(CMapDataContents* contents, void* a2, CMapData* mapData, uint32_t mapDataIdx)
{
	g_origMapDataContents_Create(contents, a2, mapData, mapDataIdx);

	// #TODO: use fast listener elision when we implement it
	fx::ResourceManager::GetCurrent()->GetComponent<fx::ResourceEventManagerComponent>()->QueueEvent2("mapDataLoaded", {}, mapData->name);
}

static bool GetEntityMapDataOwner(int entityHandle, uint32_t* mapData, uint32_t* index)
{
	if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle))
	{
		if (auto ext = entity->GetExtension<MapDataOwnerExtension>())
		{
			*mapData = ext->GetMapData();
			*index = ext->GetInternalIndex();

			return true;
		}
	}

	*mapData = 0;
	*index = 0;

	return false;
}

static int GetMapDataFromHashKey(uint32_t hashKey)
{
	static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");
	auto index = _assetStore_getIndexByKey(mapDataStore, hashKey);
	return index.id;
}

static int GetEntityDefFromMapData(int mapData, uint32_t internalIndex)
{
	// try finding the entity by guid, assuming the mapdata is instantiated
	static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");

	auto pool = (atPoolBase*)((char*)mapDataStore + 56);
	if (auto entry = pool->GetAt<char>(mapData))
	{
		auto mapDataContents = *(CMapDataContents**)(entry);
		
		// try finding the entity by guid
		for (size_t i = 0; i < mapDataContents->numEntities; i++)
		{
			if (auto entity = (fwEntity*)mapDataContents->entities[i])
			{
				if (auto ext = entity->GetExtension<MapDataOwnerExtension>())
				{
					if (ext->GetGuid() == internalIndex)
					{
						return i;
					}
				}
			}
		}

		// if not found, and this is custom, just return the index
		if (IsMapDataCustom(mapData))
		{
			if (internalIndex < mapDataContents->numEntities)
			{
				return internalIndex;
			}
		}
	}

	return -1;
}

extern void* MakeStructFromMsgPack(const char* structType, const std::map<std::string, msgpack::object>& data, void* old = nullptr, bool keep = false);

static int UpdateMapdataEntity(int mapDataIdx, int entityIdx, const char* msgData, size_t msgLen)
{
	// unpack the fwMapData
	auto obj = msgpack::unpack(msgData, msgLen);
	auto data = obj->as<std::map<std::string, msgpack::object>>();

	// try finding whether or not the entity already exists
	static auto mapDataStore = streaming::Manager::GetInstance()->moduleMgr.GetStreamingModule("ymap");

	auto pool = (atPoolBase*)((char*)mapDataStore + 56);
	if (auto entry = pool->GetAt<char>(mapDataIdx))
	{
		auto mapDataContents = *(CMapDataContents**)(entry);
		auto mapData = mapDataContents->mapData;
		if (entityIdx < mapData->entities.GetCount())
		{
			auto entityDef = mapData->entities.Get(entityIdx);
			MakeStructFromMsgPack("CEntityDef", data, entityDef, true);

			fwEntity* entity = nullptr;

			if (entityIdx < mapDataContents->numEntities)
			{
				if (entity = (fwEntity*)mapDataContents->entities[entityIdx])
				{
					rage::fwModelId id;
					auto archetype = rage::fwArchetypeManager::GetArchetypeFromHashKey(entityDef->archetypeName, id);

					entity->SetupFromEntityDef(entityDef, archetype, mapDataIdx);
				}
			}

			// support a corrective matrix
			if (auto it = data.find("matrix"); it != data.end())
			{
				auto matrixArray = it->second.as<std::vector<float>>();

				if (matrixArray.size() == 16)
				{
					entityDef->extensions.Set(entityDef->extensions.GetCount(), new CustomMatrixDef(matrixArray.data()));

					if (entity)
					{
						entity->UpdateTransform(*(Matrix4x4*)matrixArray.data(), true);
					}
				}
			}
		}
	}
	else
	{
		throw std::runtime_error("Unknown map data index.");
	}
}

static void EnableEditorRuntime()
{
	g_isEditorRuntime = true;
}

static InitFunction initFunction([]()
{
	scrBindGlobal("GET_ENTITY_MAPDATA_OWNER", &GetEntityMapDataOwner);
	scrBindGlobal("GET_MAPDATA_FROM_HASH_KEY", &GetMapDataFromHashKey);
	scrBindGlobal("GET_ENTITY_INDEX_FROM_MAPDATA", &GetEntityDefFromMapData);
	scrBindGlobal("UPDATE_MAPDATA_ENTITY", &UpdateMapdataEntity);
	scrBindGlobal("ENABLE_EDITOR_RUNTIME", &EnableEditorRuntime);

	OnKillNetworkDone.Connect([]
	{
		g_isEditorRuntime = false;
	});
});

static atArray<fwFactoryBase<rage::fwExtension>*>* rage__fwFactoryManager_rage__fwExtension___ms_Factories;
static size_t (*rage__fwMapData__GetExtensionFactory)(uint32_t);

#include <RageParser.h>

static void rage__fwEntity__InitExtensionsFromDefinition(rage::fwEntity* entity, fwEntityDef* entityDef, rage::fwArchetype* archetype, uint32_t ownerHash)
{
	for (auto& extensionDef : entityDef->extensions)
	{
		if (auto extensionParser = (rage::parStructure*)extensionDef->parser_GetStructure())
		{
			auto extensionIdx = rage__fwMapData__GetExtensionFactory(extensionParser->m_nameHash);
			auto factory = (*rage__fwFactoryManager_rage__fwExtension___ms_Factories)[extensionIdx];

			if (auto extension = factory->Get(ownerHash))
			{
				extension->InitEntityExtensionFromDefinition(extensionDef, entity);
				entity->AddExtension(extension);
			}
		}
	}
}

static HookFunction hookFunction([]()
{
	static struct : jitasm::Frontend
	{
		virtual void InternalMain() override
		{
			mov(rax, (uint64_t)&g_curEntityIndex);
			mov(dword_ptr[rax], r12d);

			mov(rax, (uint64_t)&ConstructEntity);
			jmp(rax);
		}
	} constructEntityStub;

	{
		auto location = hook::get_pattern("4D 8B C6 41 8B D7 48 8B CF E8 ? ? ? ? 49 8B 4D", 9);
		hook::set_call(&g_origConstructEntity, location);
		hook::call(location, constructEntityStub.GetCode());
	}

	{
		MH_Initialize();
		MH_CreateHook(hook::get_pattern("F7 42 24 A0 05 00 00 49 8B D8", -0x18), MapDataContents_Create, (void**)&g_origMapDataContents_Create);
		MH_EnableHook(MH_ALL_HOOKS);
	}

	// retain loaded mapdata pso
	hook::nop(hook::get_pattern("48 8B F1 74 04 48 89 59 28", 3), 2);
	hook::put<uint8_t>(hook::get_pattern("74 22 49 8D 8E B0", 0), 0xEB);

	{
		auto location = hook::get_pattern("41 F7 43 24 A0 05 00 00 0F 94 C0", 8);
		hook::nop(location, 3);
		hook::put<uint16_t>(location, 0x00B0);
	}

	// reimplement for non-factory entities (null check)
	{
		auto location = hook::get_pattern<char>("66 44 3B 62 68 73 60", -0x2C);
		hook::jump(location, rage__fwEntity__InitExtensionsFromDefinition);
		hook::set_call(&rage__fwMapData__GetExtensionFactory, location + 0x4A);
		rage__fwFactoryManager_rage__fwExtension___ms_Factories = hook::get_address<decltype(rage__fwFactoryManager_rage__fwExtension___ms_Factories)>(location + 0x57);
	}
});
