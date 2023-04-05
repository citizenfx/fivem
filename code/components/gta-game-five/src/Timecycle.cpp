#include <StdInc.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Timecycle.h>
#include <GameInit.h>

static TimecycleScriptData* g_timeCycleScriptData;

rage::tcManager* TimecycleManager::GetGameManager()
{
	assert(rage::tcManager::ms_Instance);
	return rage::tcManager::ms_Instance;
};

TimecycleScriptData* TimecycleManager::GetScriptData()
{
	return g_timeCycleScriptData;
};

std::string& TimecycleManager::GetTimecycleName(uint32_t hash)
{
	if (auto& it = m_originalNames.find(hash); it != m_originalNames.end())
	{
		return it->second;
	}

	if (auto& it = m_customNames.find(hash); it != m_customNames.end())
	{
		return it->second;
	}

	assert(false && va("Failed to find timecycle name for %X", hash));
}

std::string& TimecycleManager::GetTimecycleName(const rage::tcModifier& modifier)
{
	return GetTimecycleName(modifier.m_nameHash);
}

rage::tcVarInfo* TimecycleManager::GetConfigVarInfo(const std::string& paramName, bool search)
{
	if (strlen(paramName.c_str()) == 0)
	{
		return nullptr;
	}

	if (auto tcVarInfos = *rage::tcConfig::ms_pVarInfos)
	{
		for (int i = 0; i < GetConfigVarInfoCount(); i++)
		{
			auto& varInfo = tcVarInfos[i];

			if (search && strstr(varInfo.m_name, paramName.c_str()) != NULL)
			{
				return &varInfo;
			}
			else if (!search && strcmp(varInfo.m_name, paramName.c_str()) == NULL)
			{
				return &varInfo;
			}
		}
	}

	return nullptr;
}

rage::tcVarInfo* TimecycleManager::GetConfigVarInfo(const int paramIndex)
{
	if (auto tcVarInfos = *rage::tcConfig::ms_pVarInfos)
	{
		for (int i = 0; i < GetConfigVarInfoCount(); i++)
		{
			auto& varInfo = tcVarInfos[i];

			if (varInfo.m_index == paramIndex)
			{
				return &varInfo;
			}
		}
	}

	return nullptr;
}
rage::tcVarInfo* TimecycleManager::GetConfigVarInfos()
{
	return *rage::tcConfig::ms_pVarInfos;
}

rage::tcModData* TimecycleManager::GetTimecycleModDataByIndex(rage::tcModifier& modifier, const int index)
{
	if (index < modifier.m_modData.GetCount())
	{
		return &modifier.m_modData[index];
	}

	return nullptr;
}

rage::tcModData* TimecycleManager::GetTimecycleModData(rage::tcModifier& modifier, const std::string& paramName)
{
	if (auto tcVarInfos = *rage::tcConfig::ms_pVarInfos)
	{
		for (auto& modData : modifier.m_modData)
		{
			if (modData.m_index == -1 || modData.m_index > GetConfigVarInfoCount())
			{
				continue; // invalid?
			}

			auto& varInfo = tcVarInfos[modData.m_index];

			if (strcmp(varInfo.m_name, paramName.c_str()) == NULL)
			{
				return &modData;
			}
		}
	}

	return nullptr;
}

rage::tcModData* TimecycleManager::GetTimecycleModData(rage::tcModifier& modifier, const int paramIndex)
{
	for (auto& modData : modifier.m_modData)
	{
		if (modData.m_index == paramIndex)
		{
			return &modData;
		}
	}

	return nullptr;
}

rage::tcModifier* TimecycleManager::GetTimecycleByIndex(const uint32_t index)
{
	auto tcManager = GetGameManager();

	if (tcManager && index < tcManager->m_modifiers.GetCount())
	{
		return tcManager->m_modifiers[index];
	}

	return nullptr;
}

rage::tcModifier* TimecycleManager::GetTimecycle(const std::string& name)
{
	if (name.length() == 0)
	{
		return nullptr;
	}

	return TimecycleManager::GetTimecycle(HashString(name));
}

rage::tcModifier* TimecycleManager::GetTimecycle(const uint32_t hash)
{
	auto tcManager = GetGameManager();

	for (int i = 0; i < tcManager->m_modifiers.GetCount(); i++)
	{
		if (tcManager->m_modifiers[i]->m_nameHash == hash)
		{
			return tcManager->m_modifiers[i];
		}
	}

	return nullptr;
}

rage::tcModifier* TimecycleManager::CreateTimecycle(const std::string& newName)
{
	if (auto tcManager = rage::tcManager::ms_Instance)
	{
		auto nameHash = HashString(newName);

		for (auto modifier : tcManager->m_modifiers)
		{
			if (modifier->m_nameHash == nameHash)
			{
				return nullptr;
			}
		}

		auto created = new rage::tcModifier();
		created->m_modData = atArray<rage::tcModData>(8);
		created->m_nameHash = nameHash;
		created->m_varMap = nullptr;
		created->m_userFlags = 0;

		AddCustomTimecycleName(nameHash, newName);
		AddTimecycleToList(*created);

		return created;
	}

	return nullptr;
}

rage::tcModifier* TimecycleManager::CloneTimecycle(rage::tcModifier& modifier, const std::string& cloneName)
{
	auto tcManager = GetGameManager();
	auto nameHash = HashString(cloneName);

	for (auto modifier : tcManager->m_modifiers)
	{
		if (modifier->m_nameHash == nameHash)
		{
			return nullptr;
		}
	}

	auto clone = new rage::tcModifier();
	clone->m_modData = atArray(modifier.m_modData);
	clone->m_nameHash = nameHash;
	clone->m_varMap = nullptr; // we don't need this I believe
	clone->m_userFlags = modifier.m_userFlags;

	AddCustomTimecycleName(nameHash, cloneName);
	AddTimecycleToList(*clone);

	return clone;
}

void TimecycleManager::RevertChanges()
{
	/*
		Reverting process:
		1. Find all indexes of custom timecycles.
		2. Remove all custom timecycles from tcManager.
		3. Iterate over backups and find actually changed timecycles.
		4. Remove changed timecycles from tcManager.
		5. Restore removed timecycles from backup.
		6. Sort tcModifier map in tcManager.
		7. Clear backup map and custom names.
	*/

	auto tcManager = GetGameManager();

	std::vector<int> arrayIndexes{};
	std::vector<int> mapIndexes{};

	// find custom timecycles and save array indexes
	for (auto& entry : m_customNames)
	{
		for (int i = 0; i < tcManager->m_modifiers.GetCount(); i++)
		{
			if (tcManager->m_modifiers[i]->m_nameHash == entry.first)
			{
				arrayIndexes.push_back(i);
			}
		}

		for (int i = 0; i < tcManager->m_modifiersMap.m_array.GetCount(); i++)
		{
			if (tcManager->m_modifiersMap.m_array[i].m_hash == entry.first)
			{
				mapIndexes.push_back(i);
			}
		}
	}

	// remove custom timecycles from array and map
	for (auto index : arrayIndexes)
	{
		tcManager->m_modifiers.Remove(index);
	}

	for (auto index : mapIndexes)
	{
		tcManager->m_modifiersMap.m_array.Remove(index);
	}

	// find all modifiers from backup that actually were changed or removed
	// remove them from the game array and then restore from backup
	std::list<rage::tcModifier*> modifiersToRestore;

	for (auto& backup : m_modifiersBackup)
	{
		bool isModified = false;

		if (auto tc = GetTimecycle(backup.first))
		{
			if (tc->m_modData.GetCount() != backup.second->m_modData.GetCount())
			{
				isModified = true;
				// trace("%s: %X changed not the array size (cur %d) vs (bak %d)\n", __FUNCTION__, backup.second->m_nameHash, tc->m_modData.GetCount(), backup.second->m_modData.GetCount());
			}
			else
			{
				// search for any change in modData
				for (int i = 0; i < tc->m_modData.GetCount(); i++)
				{
					auto& current = tc->m_modData[i];
					auto& stored = backup.second->m_modData[i];

					if (current.m_index != stored.m_index || current.m_value1 != stored.m_value1 || current.m_value2 != stored.m_value2)
					{
						// trace("%s: %X changed data (cur %d %f %f) vs (was %d %f %f)\n", __FUNCTION__, backup.second->m_nameHash, current.m_index, current.m_value1, current.m_value2, stored.m_index, stored.m_value1, stored.m_value2);
						isModified = true;
						break;
					}
				}
			}
		}
		else
		{
			// there potentially can be cases when we're still storing backup of an
			// unloaded timecycle, make sure we won't restore stuff in such a scenario
			if (m_originalNames.find(backup.first) != m_originalNames.end())
			{
				// trace("%s: %X was removed\n", __FUNCTION__, backup.second->m_nameHash);
				isModified = true;
			}
		}

		if (isModified)
		{
			modifiersToRestore.push_back(backup.second);
			RemoveTimecycle(backup.first);
		}
	}

	// add original modifiers back
	for (auto backup : modifiersToRestore)
	{
		AddTimecycleToList(*backup, false);
	}

	// sort timecycle modifier map after all the mess
	SortTimecycleMap();

	// clear custom timecycle names
	m_customNames.clear();

	// clear backups map
	m_modifiersBackup.clear();
}

void TimecycleManager::SortTimecycleMap()
{
	auto tcManager = GetGameManager();

	std::sort(tcManager->m_modifiersMap.m_array.begin(), tcManager->m_modifiersMap.m_array.end(), [](const auto& left, const auto& right)
	{
		return (left.m_hash < right.m_hash);
	});

	tcManager->m_modifiersMap.m_isSorted = true;
}

void TimecycleManager::RemoveTimecycle(const uint32_t hash)
{
	auto index = GetTimecycleIndex(hash);

	if (index == -1)
	{
		return;
	}

	// ensure we're not using this timecycle when removing it
	if (auto scriptData = TimecycleManager::GetScriptData())
	{
		if (index == scriptData->m_primaryModifierIndex)
		{
			scriptData->m_primaryModifierIndex = -1;
		}

		if (index == scriptData->m_extraModifierIndex)
		{
			scriptData->m_extraModifierIndex = -1;
		}

		if (index == scriptData->m_transitionModifierIndex)
		{
			scriptData->m_transitionModifierIndex = -1;
		}
	}

	auto tcManager = GetGameManager();

	delete tcManager->m_modifiers[index];
	tcManager->m_modifiers.Remove(index);

	for (int i = 0; i < tcManager->m_modifiersMap.m_array.GetCount(); i++)
	{
		if (tcManager->m_modifiersMap.m_array[i].m_value->m_nameHash == hash)
		{
			tcManager->m_modifiersMap.m_array.Remove(i);
			break;
		}
	}

	RemoveCustomTimecycleName(hash);
}

void TimecycleManager::RemoveTimecycle(rage::tcModifier& modifier)
{
	RemoveTimecycle(modifier.m_nameHash);
}

void TimecycleManager::AddTimecycleToList(rage::tcModifier& modifier, bool sort)
{
	auto tcManager = GetGameManager();

	// expand modifiers array by 16 if reached size
	{
		if (tcManager->m_modifiers.GetCount() >= tcManager->m_modifiers.GetSize())
		{
			tcManager->m_modifiers.Expand(tcManager->m_modifiers.GetCount() + 16);
		}

		// add to the modifiers array
		tcManager->m_modifiers.Set(tcManager->m_modifiers.GetCount(), &modifier);
	}

	// add timecycle to the modifiers map and sort it
	{
		if (tcManager->m_modifiersMap.m_array.GetCount() >= tcManager->m_modifiersMap.m_array.GetSize())
		{
			tcManager->m_modifiersMap.m_array.Expand(tcManager->m_modifiersMap.m_array.GetCount() + 16);
		}

		auto dataPair = new rage::atBinaryMap<rage::tcModifier*>::DataPair();
		dataPair->m_hash = modifier.m_nameHash;
		dataPair->m_value = &modifier;

		tcManager->m_modifiersMap.m_array.Set(tcManager->m_modifiersMap.m_array.GetCount(), *dataPair);
		tcManager->m_modifiersMap.m_isSorted = false;

		if (sort)
		{
			std::sort(tcManager->m_modifiersMap.m_array.begin(), tcManager->m_modifiersMap.m_array.end(), [](const auto& left, const auto& right)
			{
				return (left.m_hash < right.m_hash);
			});

			tcManager->m_modifiersMap.m_isSorted = true;
		}
	}
}

void TimecycleManager::EnsureTimecycleBackup(const rage::tcModifier& modifier)
{
	if (IsCustomTimecycle(modifier) || IsTimecycleBackedUp(modifier))
	{
		return;
	}

	auto backup = new rage::tcModifier();
	backup->m_nameHash = modifier.m_nameHash;
	backup->m_modData = atArray<rage::tcModData>(modifier.m_modData);
	backup->m_userFlags = 0;
	backup->m_varMap = nullptr;

	m_modifiersBackup[modifier.m_nameHash] = backup;
}

bool TimecycleManager::IsTimecycleBackedUp(const rage::tcModifier& modifier)
{
	return m_modifiersBackup.find(modifier.m_nameHash) != m_modifiersBackup.end();
}

bool TimecycleManager::IsCustomTimecycle(const rage::tcModifier& modifier)
{
	return m_originalNames.find(modifier.m_nameHash) == m_originalNames.end();
}

bool TimecycleManager::HasTimecycleWithName(const std::string& name)
{
	auto tcManager = GetGameManager();
	auto nameHash = HashString(name);

	for (int i = 0; i < tcManager->m_modifiers.GetCount(); i++)
	{
		if (tcManager->m_modifiers[i]->m_nameHash == nameHash)
		{
			return true;
		}
	}

	return false;
}

void TimecycleManager::HandleTimecycleLoaded(const uint32_t hash, const std::string& name)
{
	m_originalNames[hash] = name;
}

void TimecycleManager::HandleTimecycleUnloaded(const uint32_t hash)
{
	m_originalNames.erase(hash);

	RemoveTimecycleBackup(hash);
}

void TimecycleManager::RemoveTimecycleBackup(const uint32_t hash)
{
	if (auto it = m_modifiersBackup.find(hash); it != m_modifiersBackup.end())
	{
		m_modifiersBackup.erase(it);
	}
}

void TimecycleManager::AddCustomTimecycleName(const uint32_t hash, const std::string& name)
{
	m_customNames[hash] = name;
}

void TimecycleManager::RemoveCustomTimecycleName(const uint32_t hash)
{
	m_customNames.erase(hash);
}

bool TimecycleManager::DoesTimecycleHasModData(rage::tcModifier& modifier, const std::string& paramName, bool search)
{
	if (auto varInfo = GetConfigVarInfo(paramName, search))
	{
		return DoesTimecycleHasModData(modifier, varInfo->m_index);
	}

	return false;
}

bool TimecycleManager::DoesTimecycleHasModData(rage::tcModifier& modifier, const int index)
{
	for (auto& entry : modifier.m_modData)
	{
		if (entry.m_index == index)
		{
			return true;
		}
	}

	return false;
}

bool TimecycleManager::SetTimecycleModData(rage::tcModifier& modifier, std::string& paramName, const float value1, const float value2)
{
	if (auto varInfo = GetConfigVarInfo(paramName))
	{
		return SetTimecycleModData(modifier, varInfo->m_index, value1, value2);
	}

	return false;
}

bool TimecycleManager::SetTimecycleModData(rage::tcModifier& modifier, const int index, const float value1, const float value2)
{
	EnsureTimecycleBackup(modifier);

	for (auto& entry : modifier.m_modData)
	{
		if (entry.m_index == index)
		{
			entry.m_value1 = value1;
			entry.m_value2 = value2;

			return true;
		}
	}

	return false;
}

bool TimecycleManager::AddTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData)
{
	for (auto& entry : modifier.m_modData)
	{
		if (modData.m_index == entry.m_index)
		{
			return false;
		}
	}

	EnsureTimecycleBackup(modifier);

	if (modifier.m_modData.GetCount() >= modifier.m_modData.GetSize())
	{
		modifier.m_modData.Expand(modifier.m_modData.GetSize() + 16);
	}

	modifier.m_modData.Set(modifier.m_modData.GetCount(), modData);

	std::sort(modifier.m_modData.begin(), modifier.m_modData.end(), [](const auto& left, const auto& right)
	{
		return (left.m_index < right.m_index);
	});

	return true;
}

bool TimecycleManager::AddTimecycleModData(rage::tcModifier& modifier, const std::string& paramName)
{
	if (auto varInfo = GetConfigVarInfo(paramName))
	{
		auto modData = new rage::tcModData();
		modData->m_index = varInfo->m_index;
		modData->m_value1 = varInfo->m_value;
		modData->m_value2 = 0.0f;

		return AddTimecycleModData(modifier, *modData);
	}

	return false;
}

bool TimecycleManager::RemoveTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData)
{
	EnsureTimecycleBackup(modifier);

	for (int i = 0; i < modifier.m_modData.GetCount(); i++)
	{
		if (modifier.m_modData[i].m_index == modData.m_index)
		{
			modifier.m_modData.Remove(i);
			return true;
		}
	}

	return false;
}

bool TimecycleManager::RemoveTimecycleModData(rage::tcModifier& modifier, const std::string& paramName)
{
	if (auto modData = GetTimecycleModData(modifier, paramName))
	{
		return RemoveTimecycleModData(modifier, *modData);
	}

	return false;
}

bool TimecycleManager::RenameTimecycle(rage::tcModifier& modifier, const std::string& newName)
{
	auto newHash = HashString(newName);

	if (modifier.m_nameHash == newHash)
	{
		return true;
	}

	EnsureTimecycleBackup(modifier);

	auto& modifiersMap = rage::tcManager::ms_Instance->m_modifiersMap;

	for (auto it = modifiersMap.m_array.begin(); it != modifiersMap.m_array.end(); ++it)
	{
		if (it->m_hash == modifier.m_nameHash)
		{
			it->m_hash = newHash;
			it->m_value->m_nameHash = newHash;

			modifiersMap.m_isSorted = false;

			break;
		}
	}

	if (!modifiersMap.m_isSorted)
	{
		std::sort(modifiersMap.m_array.begin(), modifiersMap.m_array.end(), [](const auto& left, const auto& right)
		{
			return (left.m_hash < right.m_hash);
		});

		modifiersMap.m_isSorted = true;
	}

	AddCustomTimecycleName(newHash, newName);

	return modifier.m_nameHash != newHash;
}

int TimecycleManager::GetTimecycleIndex(const rage::tcModifier& modifier)
{
	return GetTimecycleIndex(modifier.m_nameHash);
}

int TimecycleManager::GetTimecycleIndex(const std::string& name)
{
	auto nameHash = HashString(name);
	return GetTimecycleIndex(nameHash);
}

int TimecycleManager::GetTimecycleIndex(const uint32_t hash)
{
	auto tcManager = GetGameManager();

	for (int i = 0; i < tcManager->m_modifiers.GetCount(); i++)
	{
		if (tcManager->m_modifiers[i]->m_nameHash == hash)
		{
			return i;
		}
	}

	return -1;
}

int TimecycleManager::GetConfigVarInfoCount()
{
	return *rage::tcConfig::ms_numVars;
}

static TimecycleManager TCManager;
TimecycleManager* TheTimecycleManager = &TCManager;

static uint32_t (*g_origTimecycleComputeHashLoad)(int, char*);
static uint32_t TimecycleComputeHashLoad(int ns, char* name)
{
	auto hash = g_origTimecycleComputeHashLoad(ns, name);
	TCManager.HandleTimecycleLoaded(hash, std::string(name));
	return hash;
}

static uint32_t (*g_origTimecycleComputeHashUnload)(int, char*);
static uint32_t TimecycleComputeHashUnload(int ns, char* name)
{
	auto hash = g_origTimecycleComputeHashUnload(ns, name);
	TCManager.HandleTimecycleUnloaded(hash);
	return hash;
}

static HookFunction hookFunction([]()
{
	OnKillNetworkDone.Connect([=]()
	{
		TCManager.RevertChanges();
	});

	static_assert(sizeof(rage::tcModData) == 0xC);
	static_assert(sizeof(rage::tcVarInfo) == 0x20);
	static_assert(sizeof(rage::tcModifier) == 0x28);
	static_assert(sizeof(rage::atBinaryMap<void*>) == 0x18);

	{
		auto location = hook::get_pattern<char>("7E 6E 45 33 F6");
		rage::tcConfig::ms_numVars = hook::get_address<int*>(location - 6, 2, 6);
		rage::tcConfig::ms_pVarInfos = hook::get_address<rage::tcVarInfo**>(location + 5, 3, 7);
	}

	{
		auto location = hook::get_pattern<char>("48 C1 E0 03 41 80 E1 01 C6 44 24 20 00", 19);
		rage::tcManager::ms_Instance = hook::get_address<rage::tcManager*>(location, 3, 7);
	}

	{
		auto location = hook::get_pattern<char>("48 89 7B 10 8B 05 ? ? ? ? 83 F8 FF 74 13");
		g_timeCycleScriptData = hook::get_address<TimecycleScriptData*>(location + 6);
	}

	{
		auto location = hook::get_pattern("48 8B 50 08 E8 ? ? ? ? B9", 4);

		hook::set_call(&g_origTimecycleComputeHashLoad, location);
		hook::call(location, TimecycleComputeHashLoad);
	}

	{
		auto location = hook::get_pattern("48 8B 50 08 E8 ? ? ? ? 48 8D 54 24", 4);

		hook::set_call(&g_origTimecycleComputeHashUnload, location);
		hook::call(location, TimecycleComputeHashUnload);
	}
});
