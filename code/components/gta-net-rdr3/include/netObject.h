#pragma once

#include <NetworkPlayerMgr.h>
#include <CrossBuildRuntime.h>

enum class NetObjEntityType;

inline int MapNetObjectMethod(int offset)
{
	if (offset >= 0x170 && xbr::IsGameBuildOrGreater<1491>())
	{
		offset += 0x8;
	}

	return offset;
}

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
	uint8_t unk1 : 1;
	uint8_t unk2 : 1;
	uint8_t shouldNotBeDeleted : 1;
	char pad_3[3]; // +73;
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
	virtual ~netObject() = 0;

	CNetworkSyncDataULBase syncData; // +8

	inline netBlender* GetBlender()
	{
		return *(netBlender**)((uintptr_t)this + 96);
	}

private:
	template<typename TMember>
	inline static TMember get_member(void* ptr)
	{
		union member_cast
		{
			TMember function;
			struct
			{
				void* ptr;
				uintptr_t off;
			};
		};

		member_cast cast;
		cast.ptr = ptr;
		cast.off = 0;

		return cast.function;
	}

public:
#undef FORWARD_FUNC
#define FORWARD_FUNC(name, offset, ...)     \
	using TFn = decltype(&netObject::name); \
	void** vtbl = *(void***)(this);         \
	return (this->*(get_member<TFn>(vtbl[MapNetObjectMethod(offset) / 8])))(__VA_ARGS__);

	inline void* GetSyncData()
	{
		FORWARD_FUNC(GetSyncData, 0x20);
	}

	inline netSyncTree* GetSyncTree()
	{
		FORWARD_FUNC(GetSyncTree, 0x30);
	}

	inline bool CanSyncWithNoGameObject()
	{
		FORWARD_FUNC(CanSyncWithNoGameObject, 0x98);
	}

	inline void* GetGameObject()
	{
		FORWARD_FUNC(GetGameObject, 0xB0);
	}

	inline void* CreateNetBlender()
	{
		FORWARD_FUNC(CreateNetBlender, 0x108);
	}

	inline int GetSyncFrequency()
	{
		FORWARD_FUNC(GetSyncFrequency, 0x128);
	}

	inline void MainThreadUpdate()
	{
		FORWARD_FUNC(MainThreadUpdate, 0x160);
	}

	inline void DependencyThreadUpdate()
	{
		FORWARD_FUNC(DependencyThreadUpdate, 0x168);
	}

	inline void PostDependencyThreadUpdate()
	{
		FORWARD_FUNC(PostDependencyThreadUpdate, 0x170);
	}

	inline void StartSynchronising()
	{
		FORWARD_FUNC(StartSynchronising, 0x178);
	}

	inline bool CanClone(CNetGamePlayer* player, uint32_t* reason)
	{
		FORWARD_FUNC(CanClone, 0x188, player, reason);
	}

	inline bool CanSynchronise(bool unk, uint32_t* reason)
	{
		FORWARD_FUNC(CanSynchronise, 0x1A0, unk, reason);
	}

	inline bool CanPassControl(CNetGamePlayer* player, int type, int* outReason)
	{
		FORWARD_FUNC(CanPassControl, 0x1D0, player, type, outReason);
	}

	inline bool CanBlend(int* outReason)
	{
		FORWARD_FUNC(CanBlend, 0x1E0, outReason);
	}

	inline void ChangeOwner(CNetGamePlayer* player, int migrationType)
	{
		FORWARD_FUNC(ChangeOwner, 0x1F8, player, migrationType);
	}

	inline void OnRegistered()
	{
		FORWARD_FUNC(OnRegistered, 0x200);
	}

	inline void PostMigrate(int migrationType)
	{
		FORWARD_FUNC(PostMigrate, 0x258, migrationType);
	}

	inline void PostCreate()
	{
		FORWARD_FUNC(PostCreate, 0x240);
	}

	inline void PreSync()
	{
		FORWARD_FUNC(PreSync, 0x248);
	}

	inline void PostSync()
	{
		FORWARD_FUNC(PostSync, 0x250);
	}

	inline const char* GetTypeString()
	{
		FORWARD_FUNC(GetTypeString, 0x270);
	}

#undef FORWARD_FUNC

public:
	inline void Update()
	{
		MainThreadUpdate();
		DependencyThreadUpdate();
		PostDependencyThreadUpdate();
	}

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
