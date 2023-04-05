#pragma once
#include <StdInc.h>
#include <sysAllocator.h>
#include <atArray.h>

#ifdef COMPILING_GTA_GAME_FIVE
#define GTA_GAME_EXPORT DLL_EXPORT
#else
#define GTA_GAME_EXPORT DLL_IMPORT
#endif

namespace rage
{
// TODO: atBinaryMap proper implementation
template<typename TValue = void*>
class atBinaryMap : public rage::sysUseAllocator
{
public:
	struct DataPair : rage::sysUseAllocator
	{
		uint32_t m_hash;
		TValue m_value;
	};

public:
	bool m_isSorted;
	atArray<DataPair> m_array;
};

struct tcModData : rage::sysUseAllocator
{
	int m_index;
	float m_value1;
	float m_value2;
};

struct tcModifier : rage::sysUseAllocator
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
	char pad[4];
	char* m_name;
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
	char pad[0x40];

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
	uint32_t m_primaryModifierIndex; // (GET/SET)_TIMECYCLE_MODIFIER
	float m_primaryModifierStrength;
	uint32_t m_transitionModifierIndex; // (GET/SET)_TRANSITION_TIMECYCLE_MODIFIER
	float m_transitionModifierStrength;
	float m_transitionModifierSpeed; // second arg of (GET/SET)_TRANSITION_TIMECYCLE_MODIFIER
	char pad[24];
	uint32_t m_extraModifierIndex; // _SET_EXTRA_TIMECYCLE_MODIFIER
};

class TimecycleManager
{
private:
	std::map<uint32_t, std::string> m_originalNames; // names that were gathered from data files
	std::map<uint32_t, std::string> m_customNames; // names of custom timecycles that were created in code
	std::map<uint32_t, rage::tcModifier*> m_modifiersBackup; // backups of original modifiers

public:
	GTA_GAME_EXPORT std::string& GetTimecycleName(const uint32_t hash);
	GTA_GAME_EXPORT std::string& GetTimecycleName(const rage::tcModifier& modifier);
	GTA_GAME_EXPORT rage::tcModifier* GetTimecycle(const uint32_t hash);
	GTA_GAME_EXPORT rage::tcModifier* GetTimecycle(const std::string& name);
	GTA_GAME_EXPORT rage::tcModifier* GetTimecycleByIndex(const uint32_t index);
	GTA_GAME_EXPORT rage::tcModifier* CreateTimecycle(const std::string& newName);
	GTA_GAME_EXPORT rage::tcModifier* CloneTimecycle(rage::tcModifier& modifier, const std::string& cloneName);
	GTA_GAME_EXPORT rage::tcModData* GetTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	GTA_GAME_EXPORT rage::tcModData* GetTimecycleModData(rage::tcModifier& modifier, const int index);
	GTA_GAME_EXPORT rage::tcModData* GetTimecycleModDataByIndex(rage::tcModifier& modifier, const int index);
	GTA_GAME_EXPORT bool AddTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData);
	GTA_GAME_EXPORT bool AddTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	GTA_GAME_EXPORT bool SetTimecycleModData(rage::tcModifier& modifier, std::string& paramName, const float value1, const float value2);
	GTA_GAME_EXPORT bool SetTimecycleModData(rage::tcModifier& modifier, const int index, const float value1, const float value2);
	GTA_GAME_EXPORT bool RemoveTimecycleModData(rage::tcModifier& modifier, const rage::tcModData& modData);
	GTA_GAME_EXPORT bool RemoveTimecycleModData(rage::tcModifier& modifier, const std::string& paramName);
	GTA_GAME_EXPORT bool RenameTimecycle(rage::tcModifier& modifier, const std::string& newName);
	GTA_GAME_EXPORT bool DoesTimecycleHasModData(rage::tcModifier& modifier, const std::string& paramName, bool search = false);
	GTA_GAME_EXPORT bool DoesTimecycleHasModData(rage::tcModifier& modifier, const int index);
	GTA_GAME_EXPORT int GetTimecycleIndex(const rage::tcModifier& modifier);
	GTA_GAME_EXPORT int GetTimecycleIndex(const std::string& name);
	GTA_GAME_EXPORT int GetTimecycleIndex(const uint32_t hash);
	GTA_GAME_EXPORT void EnsureTimecycleBackup(const rage::tcModifier& modifier);
	GTA_GAME_EXPORT void RemoveTimecycle(rage::tcModifier& modifier);

public:
	void RevertChanges();
	void HandleTimecycleLoaded(const uint32_t hash, const std::string& name);
	void HandleTimecycleUnloaded(const uint32_t hash);
	void RemoveTimecycleBackup(const uint32_t hash);
	void AddCustomTimecycleName(const uint32_t hash, const std::string& name);
	void RemoveCustomTimecycleName(const uint32_t hash);
	void RemoveTimecycle(const uint32_t hash);
	bool IsTimecycleBackedUp(const rage::tcModifier& modifier);
	bool IsCustomTimecycle(const rage::tcModifier& modifier);

public:
	static GTA_GAME_EXPORT rage::tcManager* GetGameManager(); // get pointer to instance of RAGE timecycle manager
	static GTA_GAME_EXPORT TimecycleScriptData* GetScriptData(); // get "script" data, seems to be struct that is used in natives
	static GTA_GAME_EXPORT rage::tcVarInfo* GetConfigVarInfo(const std::string& paramName, bool search = false);
	static GTA_GAME_EXPORT rage::tcVarInfo* GetConfigVarInfo(const int index);
	static GTA_GAME_EXPORT rage::tcVarInfo* GetConfigVarInfos();
	static GTA_GAME_EXPORT int GetConfigVarInfoCount();
	static GTA_GAME_EXPORT bool HasTimecycleWithName(const std::string& paramName);

private:
	static void AddTimecycleToList(rage::tcModifier& modifier, bool sort = true);
	static void SortTimecycleMap();
};

extern GTA_GAME_EXPORT TimecycleManager* TheTimecycleManager;
