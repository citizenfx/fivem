#pragma once

#include <NetworkPlayerMgr.h>

enum class NetObjEntityType;

namespace rage
{
class netBlender;
class netSyncTree;

class CNetworkSyncDataULBase
{
public:
	virtual ~CNetworkSyncDataULBase() = default;

	// dummy functions to satisfy compiler
	inline virtual void m_8() { }

	inline virtual void m_10() { }

	inline virtual void m_18() { }

	inline virtual void m_20() { }

	inline virtual void m_28() { }

	inline virtual void m_30() { }

	inline virtual void m_38() { }

	inline virtual void m_40() { }

	inline virtual void m_48() { }

	inline virtual void SetCloningFrequency(int player, int frequency) { }

public:
	char pad[48]; // +16
	uint16_t objectType; // +64
	uint16_t objectId; // +66
	char pad_2[1]; // +68
	uint8_t ownerId; // +69
	uint8_t nextOwnerId; // +70
	uint8_t isRemote; // +71
	uint8_t wantsToDelete : 1; // +72
	char pad_3[3]; // +73;
	uint8_t shouldNotBeDeleted : 1; // +76
	char pad_4[3];
	char pad_5[32];
	uint32_t creationAckedPlayers; // +112
	uint32_t m64;
	uint32_t m68;
	uint32_t m6C;

public:
	inline bool IsCreationAckedByPlayer(int index)
	{
		return (creationAckedPlayers & (1 << index)) != 0;
	}
};

class netObject
{
public:
	CNetworkSyncDataULBase syncData; // +8

	inline netBlender* GetBlender()
	{
		return *(netBlender**)((uintptr_t)this + 96);
	}

	virtual ~netObject() = 0;
	virtual void m_8() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void* m_20() = 0; // GetSyncData
	virtual void m_28() = 0;
	virtual netSyncTree* GetSyncTree() = 0;
	virtual bool HasGameObject() = 0;
	virtual void m_40() = 0;
	virtual int GetClonedState() = 0; // 10
	virtual uint32_t GetLastPlayersSyncUpdated() = 0;
	virtual int GetMinimumUpdateLevel() = 0;
	virtual int GetUpdateLevel() = 0;
	virtual char* GetLogName() = 0;
	virtual bool CanApplyNodeData() = 0;
	virtual bool HasNodeDependencies(void* syncDataNode) = 0;
	virtual void* GetAllNodeDependencies(void* syncDataNode) = 0;
	virtual int GetLastSyncTreeUpdateFrame() = 0;
	virtual void SetLastSyncTreeUpdateFrame(uint32_t timestamp) = 0;
	virtual bool CanSyncWithNoGameObject() = 0; // 20
	virtual void Init() = 0;
	virtual void ForceUpdateInScopeStateThisFrame() = 0;
	virtual void* GetGameObject() = 0; // GetEntity
	virtual void SetPendingPlayerIndex(uint16_t index) = 0;
	virtual void ClearPendingPlayerIndex() = 0;
	virtual void SetGlobalFlags() = 0;
	virtual void m_D0() = 0;
	virtual int GetObjectFlags() = 0;
	virtual void RefreshLogName() = 0;
	virtual void SetOwnershipToken(int token) = 0; // 30
	virtual void SetGlobalFlagsNodeDirty(int flags) = 0;
	virtual void m_F8() = 0; // added in 1311
	virtual void SetGameObject(void* obj) = 0;
	virtual void CreateNetBlender() = 0;
	virtual void m_108() = 0;
	virtual void m_110() = 0;
	virtual void m_118() = 0;
	virtual int GetSyncFrequency() = 0; // GetDefaultUpdateLevel
	virtual void m_128() = 0;
	virtual void m_130() = 0; // 40
	virtual void m_138() = 0;
	virtual void m_140() = 0;
	virtual bool IsInScope(void* player) = 0;
	virtual void ManageUpdateLevel() = 0;
	virtual void MainThreadUpdate() = 0;
	virtual void DependencyThreadUpdate() = 0;
	virtual void PostDependencyThreadUpdate() = 0;
	virtual void StartSynchronising() = 0;
	virtual void StopSynchronising() = 0;
	virtual bool CanClone(void* player, uint32_t* reason) = 0; // 50
	virtual bool CanSync(void* player) = 0;
	virtual void m_910() = 0;
	virtual bool CanSynchronise(bool, int*) = 0;
	virtual bool CanStopSynchronize() = 0;
	virtual bool CanDelete() = 0;
	virtual bool CanCreateWithNoGameObject() = 0;
	virtual bool CanPassControlWithNoGameObject() = 0;
	virtual bool CanReassignWithNoGameObject() = 0;
	virtual bool m_158(void* player, int type, int* outReason) = 0; // CanPassControl
	virtual bool CanAcceptControl(void* player, int type, int* outReason) = 0; // 60
	virtual bool m_168(int* outReason) = 0; // CanBlend
	virtual bool NetworkBlenderIsOverridden(void* unk) = 0;
	virtual bool NetworkAttachmentIsOverridden() = 0;
	virtual void ChangeOwner(void* player, int migrationType) = 0;
	virtual void OnRegistered() = 0;
	virtual void OnUnregistered() = 0;
	virtual bool NeedsReassigning() = 0;
	virtual void OnReassigned() = 0;
	virtual void OnCloning(void* player) = 0;
	virtual int CalcReassignPriority() = 0; // 70
	virtual void PlayerHasJoined(void* player) = 0;
	virtual void PlayerHasLeft(void* player) = 0;
	virtual void m_1C0() = 0; // PostCreate
	virtual void PreSync() = 0;
	virtual void m_1D0() = 0; // PostSync
	virtual void PostMigrate(int migrationType) = 0;
	virtual void SetBlenderTimestamp(uint32_t) = 0;
	virtual bool CheckPlayerHasAuthorityOverObject(void*) = 0;
	virtual const char* GetTypeString() = 0;

	inline uint16_t GetObjectId()
	{
		return syncData.objectId;
	}

	inline uint16_t GetObjectType()
	{
		return syncData.objectType;
	}

	inline std::string ToString()
	{
		return fmt::sprintf("[netObj:%d:%s]", GetObjectId(), GetTypeString());
	}
};

netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4);
}
