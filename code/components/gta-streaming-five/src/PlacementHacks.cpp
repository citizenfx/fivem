/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>

#include <EntitySystem.h>

#include <atArray.h>
#include <fiDevice.h>

#include <rapidjson/document.h>
#include <rapidjson/reader.h>

#include <boost/preprocessor.hpp>

#include <ICoreGameInit.h>

#include <gameSkeleton.h>
#include "Streaming.h"

namespace rage
{
	static hook::cdecl_stub<void(fwRefAwareBase* self, void** ref)> _addKnownRef([]()
	{
		return hook::get_call(hook::get_pattern("74 20 48 85 C9 74 08", 29));
	});

	static hook::cdecl_stub<void(fwRefAwareBase* self, void** ref)> _removeKnownRef([]()
	{
		return hook::get_call(hook::get_pattern("74 20 48 85 C9 74 08", 10));
	});

	void fwRefAwareBase::AddKnownRef(void** ref)
	{
		return _addKnownRef(this, ref);
	}

	void fwRefAwareBase::RemoveKnownRef(void** ref)
	{
		return _removeKnownRef(this, ref);
	}
}

int64_t* g_baseArchetypeDefId;
/* #TODO2802: check?
int64_t fwArchetypeDef::GetTypeIdentifier()
{
	return *g_baseArchetypeDefId;
}
*/

//static_assert(sizeof(fwArchetypeDef) == 144, "fwArchetypeDef isn't of CBaseArchetypeDef's size...");

static hook::cdecl_stub<void(CMapData*)> _mapData_ctor([]()
{
	return hook::get_pattern("48 89 01 48 89 91 E0 00 00 00 89 91 E8 00 00 00", -0x70);
});

CMapData::CMapData()
{
	_mapData_ctor(this);
}

static hook::cdecl_stub<void*(fwEntityDef*, int fileIdx, fwArchetype* archetype, uint64_t* archetypeUnk)> fwEntityDef__instantiate([] ()
{
	return hook::get_call(hook::pattern("4C 8D 4C 24 40 4D 8B C6 41 8B D7 48 8B CF").count(1).get(0).get<void>(14));
});

static hook::cdecl_stub<fwArchetype*(uint32_t nameHash, uint64_t* archetypeUnk)> getArchetype([] ()
{
	return hook::get_call(hook::pattern("89 44 24 40 8B 4F 08 80 E3 01 E8").count(1).get(0).get<void>(10));
});

atArray<fwFactoryBase<fwArchetype>*>* g_archetypeFactories;

struct DataFileEntry
{
	char name[128];
	char pad[20];
	int32_t length;
};

DataFileEntry*(*dataFileMgr__getEntries)(void*, int);

static void* g_origVT[90];

template<int OrigIdx>
void* CustomVTWrapper(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11, void* a12)
{
	trace("called custom VT func %d\n", OrigIdx);

	auto origFunc = (void*(*)(void* a1, void* a2, void* a3, void* a4, void* a5, void* a6, void* a7, void* a8, void* a9, void* a10, void* a11, void* a12))g_origVT[OrigIdx];

	return origFunc(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10, a11, a12);
}

static hook::cdecl_stub<CMapDataContents*()> makeMapDataContents([] ()
{
	return hook::pattern("48 00 00 00 E8 ? ? ? ? 48 8B D8 48 85 C0 74 14").count(1).get(0).get<void>(-7);
});

static hook::cdecl_stub<void(CMapDataContents*, CMapData*, bool, bool)> addToScene([] ()
{
	return hook::pattern("48 83 EC 50 83 79 18 00 0F 29 70 C8 41 8A F1").count(1).get(0).get<void>(-0x18);
});

static hook::cdecl_stub<void(CMapDataContents*)> removeFromScene([] ()
{
	return hook::pattern("48 85 DB 74 4B 48 8B 5B 18 EB 0C 48").count(1).get(0).get<void>(-0x11);
});

hook::cdecl_stub<DataFileEntry*(void*, DataFileEntry*)> dataFileMgr__getNextEntry([] ()
{
	return hook::pattern("48 89 5C 24 08 0F B7 41 08 44 8B 82 94").count(1).get(0).get<void>();
});

static hook::cdecl_stub<void(fwArchetype*)> registerArchetype([]()
{
	return hook::get_pattern("48 8B D9 8A 49 60 80 F9", -11);
});

fwArchetype* GetArchetypeSafe(uint32_t archetypeHash, uint64_t* archetypeUnk)
{
	__try
	{
		return getArchetype(archetypeHash, archetypeUnk);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
		return nullptr;
	}
}

static std::vector<CMapDataContents*> g_sceneContentsList;
static uintptr_t sceneNodeThing;

void ParseArchetypeFile(char* text, size_t length)
{
	// null-terminate the input string
	text[length] = '\0';

	// parse a document
	rapidjson::Document document;
	document.Parse(text);

	if (document.HasParseError())
	{
		trace("parsing failed: %d\n", document.GetParseError());
		return;
	}

	rapidjson::Value entry;

	auto findMember = [&] (const char* name, rapidjson::Value& value)
	{
		auto intIt = entry.FindMember(name);

		if (intIt == entry.MemberEnd())
		{
			return false;
		}

		value = std::move(intIt->value);
		return true;
	};

	auto getVector = [&] (const char* name, float out[])
	{
		rapidjson::Value value;

		if (findMember(name, value))
		{
			if (value.IsArray())
			{
				if (value.Size() >= 3)
				{
					out[0] = value[(rapidjson::SizeType)0].GetDouble();
					out[1] = value[(rapidjson::SizeType)1].GetDouble();
					out[2] = value[(rapidjson::SizeType)2].GetDouble();

					if (value.Size() >= 4)
					{
						out[3] = value[(rapidjson::SizeType)3].GetDouble();
					}

					return true;
				}
			}
		}

		return false;
	};

	auto getInt = [&](const char* name, int* out)
	{
		rapidjson::Value value;

		if (findMember(name, value))
		{
			if (value.IsInt())
			{
				*out = value.GetInt();

				return true;
			}
		}

		return false;
	};

	auto getUInt = [&](const char* name, uint32_t* out)
	{
		rapidjson::Value value;

		if (findMember(name, value))
		{
			if (value.IsInt())
			{
				*out = value.GetInt();

				return true;
			}
		}

		return false;
	};

	auto getFloat = [&] (const char* name, float* out)
	{
		rapidjson::Value value;

		if (findMember(name, value))
		{
			if (value.IsDouble())
			{
				*out = value.GetDouble();

				return true;
			}
		}

		return false;
	};

	auto getString = [&] (const char* name, std::string* out)
	{
		rapidjson::Value value;

		if (findMember(name, value))
		{
			if (value.IsString())
			{
				*out = value.GetString();

				return true;
			}
		}

		return false;
	};

	if (document.IsObject())
	{
		auto archetypesIt = document.FindMember("archetypes");

		if (archetypesIt != document.MemberEnd())
		{
			auto& archetypes = archetypesIt->value;

			if (!archetypes.IsArray())
			{
				return;
			}

			for (auto it = archetypes.Begin(); it != archetypes.End(); it++)
			{
				entry = *it;

				if (entry.IsObject())
				{
					float aabbMin[3];
					float aabbMax[3];
					float centroid[3];
					float radius;
					float drawDistance = 300.0f;

					std::string modelName;
					std::string txdName;
					std::string parentName;

					bool valid = true;
					valid = valid && getVector("aabbMin", aabbMin);
					valid = valid && getVector("aabbMax", aabbMax);
					valid = valid && getVector("centroid", centroid);
					valid = valid && getFloat("radius", &radius);

					getFloat("drawDistance", &drawDistance);

					valid = valid && getString("archetypeName", &modelName);
					valid = valid && getString("txdName", &txdName);

					getString("lodDictName", &parentName);

					// create the archetype
					if (valid)
					{
						fwArchetypeDef* archetypeDef = new fwArchetypeDef();
						archetypeDef->lodDist = drawDistance;

						archetypeDef->bbMin[0] = aabbMin[0];
						archetypeDef->bbMin[1] = aabbMin[1];
						archetypeDef->bbMin[2] = aabbMin[2];

						archetypeDef->bbMax[0] = aabbMax[0];
						archetypeDef->bbMax[1] = aabbMax[1];
						archetypeDef->bbMax[2] = aabbMax[2];

						archetypeDef->bsCentre[0] = centroid[0];
						archetypeDef->bsCentre[1] = centroid[1];
						archetypeDef->bsCentre[2] = centroid[2];

						archetypeDef->bsRadius = radius;

						archetypeDef->name = HashString(modelName.c_str());
						archetypeDef->textureDictionary = HashString(txdName.c_str());

						if (strcmp(txdName.c_str(), "null") == 0)
						{
							archetypeDef->textureDictionary = archetypeDef->name;
						}

						if (!parentName.empty())
						{
							archetypeDef->drawableDictionary = HashString(parentName.c_str());
						}

						// assume this is a CBaseModelInfo
						// TODO: get [mi] from [miPtr]
						void* miPtr = g_archetypeFactories->Get(1)->GetOrCreate(archetypeDef->name, 1);

						fwArchetype* mi = g_archetypeFactories->Get(1)->Get(archetypeDef->name);

						mi->InitializeFromArchetypeDef(1390, archetypeDef, true);

						// TODO: clean up
						mi->flags &= ~(1 << 31);

						// register the archetype in the streaming module
						registerArchetype(mi);
					}
					else
					{
						trace("IDE_FILE archetype %s is invalid...\n", modelName.c_str());
					}
				}
			}
		}

		auto entitiesIt = document.FindMember("entities");

		if (entitiesIt != document.MemberEnd())
		{
			auto& entities = entitiesIt->value;

			if (!entities.IsArray())
			{
				return;
			}

			float aabbMin[3];
			float aabbMax[3];

			aabbMin[0] = FLT_MAX;
			aabbMin[1] = FLT_MAX;
			aabbMin[2] = FLT_MAX;

			aabbMax[0] = 0.0f - FLT_MAX;
			aabbMax[1] = 0.0f - FLT_MAX;
			aabbMax[2] = 0.0f - FLT_MAX;


			CMapDataContents* contents = makeMapDataContents();
			contents->entities = new void*[entities.Size()];
			memset(contents->entities, 0, sizeof(void*) * entities.Size());

			contents->numEntities = entities.Size();

			int i = 0;

			for (auto it = entities.Begin(); it != entities.End(); it++)
			{
				entry = *it;

				float position[3];
				float rotation[4];
				std::string guid;
				std::string archetypeName;

				bool valid = true;
				valid = valid && getVector("position", position);
				valid = valid && getVector("rotation", rotation);
				valid = valid && getString("guid", &guid);
				valid = valid && getString("archetypeName", &archetypeName);

				if (valid)
				{
					uint32_t archetypeHash = HashString(archetypeName.c_str());
					uint32_t guidHash = HashString(guid.c_str());

					if (_strnicmp(archetypeName.c_str(), "hash:", 5) == 0)
					{
						archetypeHash = _atoi64(&(archetypeName.c_str())[5]);
					}

					uint64_t archetypeUnk = 0xFFFFFFF;
					fwArchetype* archetype = GetArchetypeSafe(archetypeHash, &archetypeUnk);

					if (archetype)
					{
						fwEntityDef* entityDef = new fwEntityDef();
						entityDef->archetypeName = archetypeHash;
						entityDef->guid = guidHash;

						entityDef->position[0] = position[0];
						entityDef->position[1] = position[1];
						entityDef->position[2] = position[2];

						entityDef->rotation[0] = rotation[0];
						entityDef->rotation[1] = rotation[1];
						entityDef->rotation[2] = rotation[2];
						entityDef->rotation[3] = rotation[3];

						getFloat("float1", &entityDef->lodDist);
						getFloat("float2", &entityDef->childLodDist);

						getUInt("flags", &entityDef->flags);

						void* entity = fwEntityDef__instantiate(entityDef, 0, archetype, &archetypeUnk);

						contents->entities[i] = entity;

						// update AABB
						float xMin = position[0] - archetype->radius;
						float yMin = position[1] - archetype->radius;
						float zMin = position[2] - archetype->radius;

						float xMax = position[0] + archetype->radius;
						float yMax = position[1] + archetype->radius;
						float zMax = position[2] + archetype->radius;

						aabbMin[0] = (xMin < aabbMin[0]) ? xMin : aabbMin[0];
						aabbMin[1] = (yMin < aabbMin[1]) ? yMin : aabbMin[1];
						aabbMin[2] = (zMin < aabbMin[2]) ? zMin : aabbMin[2];

						aabbMax[0] = (xMax > aabbMax[0]) ? xMax : aabbMax[0];
						aabbMax[1] = (yMax > aabbMax[1]) ? yMax : aabbMax[1];
						aabbMax[2] = (zMax > aabbMax[2]) ? zMax : aabbMax[2];
					}
					else
					{
						trace("Couldn't find archetype %s\n", archetypeName);
					}
				}
				else
				{
					trace("IDE_FILE entity is invalid...\n");
				}

				i++;
			}

			trace("adding to scene...\n");

			CMapData mapData;
			mapData.entitiesExtentsMax[0] = aabbMax[0];
			mapData.entitiesExtentsMax[1] = aabbMax[1];
			mapData.entitiesExtentsMax[2] = aabbMax[2];
			mapData.entitiesExtentsMax[3] = FLT_MAX;

			mapData.entitiesExtentsMin[0] = aabbMin[0];
			mapData.entitiesExtentsMin[1] = aabbMin[1];
			mapData.entitiesExtentsMin[2] = aabbMin[2];
			mapData.entitiesExtentsMin[3] = 0.0f - FLT_MAX;

			mapData.contentFlags = 2;

			addToScene(contents, &mapData, false, false);
			
			g_sceneContentsList.push_back(contents);
		}
	}
}

static std::vector<std::string> g_itypRequests;
static std::vector<std::string> g_jsonRequests;

void IterateDataFiles(void* dataFileMgr, int type, std::vector<std::string>& list)
{
	DataFileEntry* entry = dataFileMgr__getEntries(dataFileMgr, type);

	while (entry->length >= 0)
	{
		list.push_back(entry->name);

		entry = dataFileMgr__getNextEntry(dataFileMgr, entry);
	}
}

void LoadArchetypeFiles()
{
	std::vector<char> fileBuffer;

	for (const std::string& entry : g_jsonRequests)
	{
		rage::fiDevice* device = rage::fiDevice::GetDevice(entry.c_str(), true);

		if (device)
		{
			uint64_t handle = device->Open(entry.c_str(), true);

			if (handle != -1)
			{
				int length = device->GetFileLength(handle);

				if (fileBuffer.size() < length)
				{
					fileBuffer.resize(length + 1);
				}

				size_t readLength = device->Read(handle, &fileBuffer[0], length);

				device->Close(handle);

				trace("parsing archetype %s...\n", entry);

				ParseArchetypeFile(&fileBuffer[0], length);

				trace("done!\n");
			}
			else
			{
				trace("failed to open %s: device returned vfs::InvalidHandle\n", entry);
			}
		}
		else
		{
			trace("failed to open %s: no device\n", entry);
		}
	}
}

static void* g_dataFileMgr;

static void* DoBeforeGetEntries(void* dataFileMgr, int type)
{
	g_dataFileMgr = dataFileMgr;

	IterateDataFiles(dataFileMgr, 1, g_jsonRequests);
	IterateDataFiles(dataFileMgr, 4, g_itypRequests);

	return dataFileMgr__getEntries(dataFileMgr, type);
}

namespace streaming
{
	void AddDataFileToLoadList(const std::string& type, const std::string& path);
}

static void RunCompatibilityBehavior()
{
	// turn PERMANENT_ITYP_FILE into DLC_ITYP_REQUEST
	bool requestedItyp = false;

	for (auto& entry : g_itypRequests)
	{
		if (strstr(entry.c_str(), "platform:/") == nullptr)
		{
			streaming::AddDataFileToLoadList("DLC_ITYP_REQUEST", entry);

			requestedItyp = true;
		}
	}

	// load archetype files
	LoadArchetypeFiles();

	// compatibility behavior!
	if (!g_jsonRequests.empty() || requestedItyp)
	{
		streaming::AddDataFileToLoadList("CFX_PSEUDO_ENTRY", "RELOAD_MAP_STORE");
	}
}

static HookFunction hookFunction([] ()
{
	/*
	ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();
	gameInit->OnGameRequestLoad.Connect([] ()
	{
		for (auto& contents : g_sceneContentsList)
		{
			removeFromScene(contents);
		}

		g_sceneContentsList.clear();
	});
	*/

	char* creator = hook::pattern("48 8B 0C C8 48 8B 01 FF 50 08 41 B1 01 4C").count(1).get(0).get<char>(-4);

	g_archetypeFactories = (decltype(g_archetypeFactories))(creator + *(int32_t*)creator + 4);

	creator = hook::pattern("8D 57 FA 8B 48 08 E8").count(1).get(0).get<char>(-4);

	g_baseArchetypeDefId = (int64_t*)(creator + *(int32_t*)creator + 4);

	void* getEntries = hook::pattern("BA 03 00 00 00 E8 ? ? ? ? 45 33 E4 B9 20").count(1).get(0).get<void>(5);
	hook::set_call(&dataFileMgr__getEntries, getEntries);
	hook::call(getEntries, DoBeforeGetEntries);

	// rename IDE_FILE to ARCHETYPE_FILE
	uint32_t* ideFile = hook::pattern("E3 94 FB CB 01 00 00 00").count(1).get(0).get<uint32_t>();
	*ideFile = HashRageString("ARCHETYPE_FILE");

	// ignore distance scale in determining whether or not to alphaclip a model
	hook::nop(hook::pattern("44 3B C1 73 3F 83").count(1).get(0).get<void>(3), 2);

	// don't lock creation of anonymous archetypes at all
	hook::put<uint8_t>(hook::get_pattern("00 41 8A E8 8B F2 48 8B D9 74", 9), 0xEB);

	// compatibility
	rage::OnInitFunctionEnd.Connect([](rage::InitFunctionType type)
	{
		if (type == rage::INIT_SESSION)
		{
			RunCompatibilityBehavior();
		}
	}, 5000);

	// yolo
	//hook::nop(hook::pattern("0F 50 C0 83 E0 07 3C 07 0F 94 C1 85 D1 74 43").count(1).get(0).get<void>(13), 2);

	//__debugbreak();
});

static size_t g_seatManagerOffset;

VehicleSeatManager* CVehicle::GetSeatManager()
{
	return reinterpret_cast<VehicleSeatManager*>(reinterpret_cast<char*>(this) + g_seatManagerOffset);
}

static HookFunction hookFunctionSeatManager([]()
{
	g_seatManagerOffset = *hook::get_pattern<uint32_t>("4C 8D B7 ? ? ? ? 41 8B DD 45 38 2E 7E 32", 3);
});
