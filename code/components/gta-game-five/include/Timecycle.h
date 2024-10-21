#pragma once
#include "ComponentExport.h"

#include <sysAllocator.h>
#include <atArray.h>

#if GTA_FIVE
#define GAME_COMPONENT_EXPORT COMPONENT_EXPORT(GTA_GAME_FIVE)
#elif IS_RDR3
#define GAME_COMPONENT_EXPORT COMPONENT_EXPORT(GTA_GAME_RDR3)
#endif

namespace rage
{
// TODO: atBinaryMap proper implementation
template<typename TValue = void*>
class atBinaryMap : public sysUseAllocator
{
public:
	struct DataPair : sysUseAllocator
	{
		uint32_t m_hash;
		TValue m_value;
	};

public:
	bool m_isSorted;
	atArray<DataPair> m_array;
};

struct tcModData : sysUseAllocator
{
	int m_index;
	float m_value1;
	float m_value2;
};

struct tcModifier : sysUseAllocator
{
	atArray<tcModData> m_modData;
	uint32_t m_nameHash;
	atArray<tcModData*>* m_varMap; // +24, seems to be related to "rage::tcModifier::RemoveVarMap"
	uint32_t m_userFlags; // +32, attribute from xml
	uint32_t m_unk;
};

struct tcVarInfo
{
	int m_index;

#if GTA_FIVE
	char pad[4];
	char* m_name;
#elif IS_RDR3
	uint32_t m_nameHash;
#endif

	float m_value;
	char end[12];
};

struct tcConfig // see "rage::tcManager::Init"
{
	static int* ms_numVars;
	static tcVarInfo** ms_pVarInfos;
};

class tcManager
{
private:
	char pad[8];

public:
	atArray<float> m_modifierStrengths;

private:
	char pad18[0x28];

public:
	atArray<tcModifier*> m_modifiers;
	atBinaryMap<tcModifier*> m_modifiersMap; // map for fast lookup?
	// etc...

public:
	static tcManager* ms_Instance;
};

int* tcConfig::ms_numVars;
tcVarInfo** tcConfig::ms_pVarInfos;
tcManager* tcManager::ms_Instance;
}

struct TimecycleScriptData // probably not even a struct, but seems consistent between game builds
{
	int m_primaryModifierIndex; // (GET/SET)_TIMECYCLE_MODIFIER
	float m_primaryModifierStrength;
	int m_transitionModifierIndex; // (GET/SET)_TRANSITION_TIMECYCLE_MODIFIER
	float m_transitionModifierStrength;
	float m_transitionModifierSpeed; // second arg of (GET/SET)_TRANSITION_TIMECYCLE_MODIFIER
#ifdef GTA_FIVE
	char pad[24];
	int m_extraModifierIndex; // _SET_EXTRA_TIMECYCLE_MODIFIER
#endif
};

class GAME_COMPONENT_EXPORT TimecycleManager
{
private:
	bool m_activateEditor; // hack...
	std::map<uint32_t, std::string> m_originalNames; // names that were gathered from data files
	std::map<uint32_t, std::string> m_customNames; // names of custom timecycles that were created in code
	std::map<uint32_t, rage::tcModifier*> m_modifiersBackup; // backups of original modifiers

#if IS_RDR3
	static std::map<uint32_t, std::string> ms_varInfoNames; // all stored param name strings
	static std::set<std::string&> ms_varInfoNameByIndex;
#endif
public:
	static TimecycleScriptData* ms_scriptData;

public:
	const std::string& GetTimecycleName(const rage::tcModifier& modifier);
	rage::tcModifier* GetTimecycle(uint32_t hash);
	rage::tcModifier* GetTimecycle(const std::string& name);
	rage::tcModifier* GetTimecycleByIndex(uint32_t index);
	rage::tcModifier* CreateTimecycle(const std::string& newName);
	rage::tcModifier* CloneTimecycle(rage::tcModifier& modifier, const std::string& cloneName);
	rage::tcModData* GetTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	rage::tcModData* GetTimecycleModData(rage::tcModifier& modifier, int index);
	rage::tcModData* GetTimecycleModDataByIndex(rage::tcModifier& modifier, int index);
	bool AddTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	bool SetTimecycleModData(rage::tcModifier& modifier, std::string& paramName, float value1, float value2);
	bool RemoveTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData);
	bool RemoveTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	bool RenameTimecycle(rage::tcModifier& modifier, const std::string& newName);
	bool DoesTimecycleHasModData(rage::tcModifier& modifier, const std::string& paramName, bool search = false);
	int GetTimecycleIndex(const rage::tcModifier& modifier);
	void EnsureTimecycleBackup(const rage::tcModifier& modifier);
	void RemoveTimecycle(rage::tcModifier& modifier);
	void RevertChanges();
	void HandleTimecycleLoaded(uint32_t hash, const std::string& name);
	void HandleTimecycleUnloaded(uint32_t hash);
	void SetActivateEditor(bool flag);
	bool ShouldActivateEditor();

	static rage::tcManager* GetGameManager(); // get pointer to instance of RAGE timecycle manager
	static TimecycleScriptData* GetScriptData(); // get "script" data, seems to be struct that is used in natives
	static rage::tcVarInfo* GetConfigVarInfo(const std::string& paramName, bool search = false);
	static rage::tcVarInfo* GetConfigVarInfo(int index);
	static rage::tcVarInfo* GetConfigVarInfos();
	static int GetConfigVarInfoCount();
	static bool HasTimecycleWithName(const std::string& paramName);
	static const char* GetVarInfoName(const rage::tcVarInfo& varInfo);

#if IS_RDR3
	static void StoreVarInfoName(const std::string& name);
#endif

private:
	const std::string& GetTimecycleName(uint32_t hash);
	bool AddTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData);
	bool DoesTimecycleHasModData(rage::tcModifier& modifier, int index);
	bool IsTimecycleBackedUp(const rage::tcModifier& modifier);
	bool IsCustomTimecycle(const rage::tcModifier& modifier);
	bool SetTimecycleModData(rage::tcModifier& modifier, int index, float value1, float value2);
	int GetTimecycleIndex(const std::string& name);
	int GetTimecycleIndex(uint32_t hash);
	void AddCustomTimecycleName(uint32_t hash, const std::string& name);
	void RemoveCustomTimecycleName(uint32_t hash);
	void RemoveTimecycle(uint32_t hash);
	void RemoveTimecycleBackup(uint32_t hash);

	static void AddTimecycleToList(rage::tcModifier& modifier, bool sort = true);
	static void FinishModifierLoad();
};

extern GAME_COMPONENT_EXPORT TimecycleManager* TheTimecycleManager;

#undef GAME_COMPONENT_EXPORT
