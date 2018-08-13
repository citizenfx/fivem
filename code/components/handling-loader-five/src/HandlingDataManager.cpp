#include "StdInc.h"

#include <HandlingLoader.h>
#include "Hooking.h"

#include <IteratorView.h>

#include <atHashMap.h>

#include <mutex>
#include <unordered_set>

static void** g_dataFileMounters;

struct DataFileEntry
{
	char name[128];
};

static bool UnloadHandlingFile(const char* handlingPath);

static bool UnloadHandlingFileEntry(void* mounter, const DataFileEntry* entry)
{
	return UnloadHandlingFile(entry->name);
}

static void PatchHandlingMounter()
{
	void* mounter = g_dataFileMounters[6];
	void** vmt = *(void***)mounter;

	vmt[2] = UnloadHandlingFileEntry;
}

class Parser;

static hook::cdecl_stub<bool(Parser*, const char*, const char*, void*, void*, bool, void*)> g_parseFileIntoStructure([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 0F B7 44 24 58 4C 8D 2D", 0));
});

static hook::cdecl_stub<void(CHandlingData*)> _processHandlingDataEntry([]()
{
	return hook::get_pattern("48 8B F9 8B 89 ? 01 00 00 0F", -10);
});

class Parser
{
public:
	bool ParseFileIntoStructure(const char* fileName, const char* type, void* dtd, void* out, bool a5, void* a6)
	{
		return g_parseFileIntoStructure(this, fileName, type, dtd, out, a5, a6);
	}
};

void CHandlingData::ProcessEntry()
{
	_processHandlingDataEntry(this);

	for (CBaseSubHandlingData* subHandling : m_subHandlingData)
	{
		if (subHandling)
		{
			subHandling->ProcessOnLoad();
		}
	}
}

static Parser** g_parser;
static void** HandlingData_DTD;

static std::unordered_map<uint32_t, std::list<int>> g_handlingStack;
static std::unordered_multimap<std::string, int> g_handlingByFile;

extern atArray<CHandlingData*>* g_handlingData;

static int FindFreeHandlingData()
{
	for (int i = 0; i < g_handlingData->GetCount(); i++)
	{
		if (!g_handlingData->Get(i))
		{
			return i;
		}
	}

	return g_handlingData->GetCount();
}

static atArray<void*>* g_archetypeFactories;

class CVehicleModelInfo
{
public:
	virtual ~CVehicleModelInfo() = default;

	// NOTE: 505 SPECIFIC
	// 1103 now
	// 1290 now
	// 1365 as well
	char pad[1200]; // +8
	int handlingDataIndex; // +1208

	char pad2[228];
};

template<typename TSubClass>
class fwFactoryBase
{
public:
	virtual ~fwFactoryBase() = 0;
};

template<typename T>
class fwArchetypeDynamicFactory : public fwFactoryBase<T>
{
private:
	atMultiHashMap<T> m_data;

public:
	void ForAllArchetypes(const std::function<void(T*)>& cb)
	{
		m_data.ForAllEntries([&](atArray<T>* arr)
		{
			for (T& mi : *arr)
			{
				cb(&mi);
			}
		});
	}
};

template<typename TSet>
static void ModifyHandlingForVehicles(const TSet& changedHandlings)
{
	auto vehicleModelInfoFactory = reinterpret_cast<fwArchetypeDynamicFactory<CVehicleModelInfo>*>(g_archetypeFactories->Get(5));

	vehicleModelInfoFactory->ForAllArchetypes([&](CVehicleModelInfo* modelInfo)
	{
		int idx = modelInfo->handlingDataIndex;

		if (idx >= 0)
		{
			if (g_handlingData->GetCount())
			{
				auto handlingData = g_handlingData->Get(idx);

				if (handlingData)
				{
					auto name = handlingData->GetName();

					if (changedHandlings.find(name) != changedHandlings.end())
					{
						const auto& stack = g_handlingStack[name];

						if (stack.empty())
						{
							trace("ModifyHandlingForVehicles: %08x is without handling!\n", name);
						}
						else
						{
							modelInfo->handlingDataIndex = stack.front();
						}
					}
				}
			}
		}
	});
}

static bool LoadHandlingFile(const char* handlingPath)
{
	atArray<CHandlingData*> handlingDataList;

	if (!(*g_parser)->ParseFileIntoStructure(handlingPath, "meta", *HandlingData_DTD, &handlingDataList, true, nullptr))
	{
		return false;
	}

	std::unordered_set<uint32_t> changedHandlings;

	trace("Loading %d handling entries from %s\n", handlingDataList.GetCount(), handlingPath);

	for (CHandlingData* handling : handlingDataList)
	{
		handling->ProcessEntry();

		int idx = FindFreeHandlingData();
		g_handlingData->Set(idx, handling);

		g_handlingStack[handling->GetName()].push_front(idx);

		changedHandlings.insert(handling->GetName());

		g_handlingByFile.insert({ handlingPath, idx });
	}

	static_assert(sizeof(CVehicleModelInfo) == 1440, "CVehicleModelInfo size");

	// override CVehicleModelInfo entries that already have a handling assigned
	ModifyHandlingForVehicles(changedHandlings);

	static std::once_flag of;
	std::call_once(of, PatchHandlingMounter);

	return true;
}

static bool UnloadHandlingFile(const char* handlingPath)
{
	std::unordered_set<uint32_t> changedHandlings;
	std::string handlingPathStr = handlingPath;

	for (auto entry : fx::GetIteratorView(g_handlingByFile.equal_range(handlingPathStr)))
	{
		uint16_t idx = entry.second;
		auto handlingData = g_handlingData->Get(idx);

		changedHandlings.insert(handlingData->GetName());

		// erase existing stack entry
		auto& hashData = g_handlingStack[handlingData->GetName()];

		for (auto it = hashData.begin(); it != hashData.end(); ++it)
		{
			if (*it == idx)
			{
				it = hashData.erase(it);
			}
		}
	}

	// edit modelinfos
	ModifyHandlingForVehicles(changedHandlings);

	// delete the handling entries
	for (auto entry : fx::GetIteratorView(g_handlingByFile.equal_range(handlingPathStr)))
	{
		if (g_handlingData->GetCount())
		{
			uint16_t idx = entry.second;
			auto handlingData = g_handlingData->Get(idx);

			if (handlingData)
			{
				g_handlingData->Set(idx, nullptr);

				auto& subHandling = handlingData->GetSubHandlingData();

				for (int i = 0; i < subHandling.GetCount(); i++)
				{
					delete subHandling.Get(i);
					subHandling.Set(i, nullptr);
				}

				delete handlingData;
			}
		}
	}

	g_handlingByFile.erase(handlingPathStr);

	return true;
}

static int GetHandlingIndexByHash(uint32_t* hash)
{
	auto it = g_handlingStack.find(*hash);

	if (it == g_handlingStack.end() || it->second.empty())
	{
		trace("Couldn't find handling data for hash 0x%08x\n", *hash);
		return -1;
	}

	return it->second.front();
}

static void LoadHandlingFileWrap(const char* handlingFile)
{
	if (!LoadHandlingFile(handlingFile))
	{
		trace("Failed to load handling file %s\n", handlingFile);
	}
}

static void LoadHandlingFileWrap2(const char* handlingFile)
{
	if (!LoadHandlingFile(handlingFile))
	{
		trace("Failed to load DLC handling file %s\n", handlingFile);
	}
}

static HookFunction hookFunction([]()
{
	{
		char* location = hook::get_pattern<char>("4C 8B E0 4C 89 6D F0 44 89 6D F8 C6", -19);
		g_parser = hook::get_address<Parser**>(location);

		location -= 7;

		HandlingData_DTD = hook::get_address<void**>(location);
	}

	// non-DLC handling loading
	hook::jump(hook::get_pattern("48 8B D0 48 8B D8 C6 44 24 20 01 E8", -0x2D), LoadHandlingFileWrap);

	// DLC handling loading
	hook::jump(hook::get_pattern("4C 8B E0 4C 89 6D F0 44  89 6D F8 C6 44 24 20 01", -0x48), LoadHandlingFileWrap2);

	// handling name get-by-hash
	hook::jump(hook::get_pattern("44 8B C2 44 8B C8 85 C0 7E 1E 44", -9), GetHandlingIndexByHash);

	// dfm
	{
		char* location = hook::get_pattern<char>("48 63 82 90 00 00 00 49 8B 8C C0 ? ? ? ? 48", 11);

		g_dataFileMounters = (decltype(g_dataFileMounters))(0x140000000 + *(int32_t*)location); // why is this an RVA?!
	}

	// archetype factories
	char* creator = hook::pattern("48 8B 0C C8 48 8B 01 FF 50 08 41 B1 01 4C").count(1).get(0).get<char>(-4);

	g_archetypeFactories = hook::get_address<decltype(g_archetypeFactories)>(creator);
});
