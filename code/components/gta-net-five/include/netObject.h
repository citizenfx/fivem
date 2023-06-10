#pragma once

#include <NetworkPlayerMgr.h>

enum class NetObjEntityType;

template<int Offset>
inline int MapNetObjectMethod()
{
	int offset = Offset;

	if constexpr (Offset >= 0x8)
	{
		if (xbr::IsGameBuildOrGreater<2802>())
		{
			offset += 0x30;
		}
	}

	if constexpr (Offset >= 0x78)
	{
		if (xbr::IsGameBuildOrGreater<2545>())
		{
			offset += 0x10;
		}
	}

	if constexpr (Offset >= 0x330)
	{
		if (xbr::IsGameBuildOrGreater<2189>())
		{
			offset += 0x8;
		}
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

	inline virtual void m_50() { }

	inline virtual void SetCloningFrequency(int player, int frequency) { }

public:
	uint8_t pad_10h[49];
	uint8_t ownerId;
	uint8_t nextOwnerId;
	uint8_t isRemote;
	uint8_t wantsToDelete : 1; // netobj+76
	uint8_t unk1 : 1;
	uint8_t shouldNotBeDeleted : 1;
	uint8_t pad_4Dh[3];
	uint8_t pad_50h[32];
	uint32_t creationAckedPlayers; // netobj+112
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

public:
	uint16_t objectType;
	uint16_t objectId;
	uint32_t pad;

	CNetworkSyncDataULBase syncData;

	inline netBlender* GetBlender()
	{
		return *(netBlender**)((uintptr_t)this + 88);
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
	return (this->*(get_member<TFn>(vtbl[MapNetObjectMethod<offset>() / 8])))(__VA_ARGS__);

	inline void* GetSyncData()
	{
		FORWARD_FUNC(GetSyncData, 0x20);
	}

	inline netSyncTree* GetSyncTree()
	{
		FORWARD_FUNC(GetSyncTree, 0x30);
	}

	inline void* GetGameObject()
	{
		FORWARD_FUNC(GetGameObject, 0x80);
	}

	inline void* CreateNetBlender()
	{
		FORWARD_FUNC(CreateNetBlender, 0xB0);
	}

	inline int GetSyncFrequency()
	{
		FORWARD_FUNC(GetSyncFrequency, 0xD0);
	}

	inline void Update()
	{
		FORWARD_FUNC(Update, 0x100);
	}

	inline void StartSynchronising()
	{
		FORWARD_FUNC(StartSynchronising, 0x110);
	}

	inline bool CanSynchronise(bool unk)
	{
		FORWARD_FUNC(CanSynchronise, 0x130, unk);
	}

	inline bool CanPassControl(CNetGamePlayer* player, int type, int* outReason)
	{
		FORWARD_FUNC(CanPassControl, 0x160, player, type, outReason);
	}

	inline bool CanBlend(int* outReason)
	{
		FORWARD_FUNC(CanBlend, 0x170, outReason);
	}

	inline void ChangeOwner(CNetGamePlayer* player, int migrationType)
	{
		FORWARD_FUNC(ChangeOwner, 0x188, player, migrationType);
	}

	inline void OnRegistered()
	{
		FORWARD_FUNC(OnRegistered, 0x190);
	}

	inline void PostMigrate(int migrationType)
	{
		FORWARD_FUNC(PostMigrate, 0x1E0, migrationType);
	}

	inline void PostCreate()
	{
		FORWARD_FUNC(PostCreate, 0x1C8);
	}

	inline void PostSync()
	{
		FORWARD_FUNC(PostSync, 0x1D8);
	}

	inline const char* GetTypeString()
	{
		FORWARD_FUNC(GetTypeString, 0x1F8);
	}

	inline void UpdatePendingVisibilityChanges()
	{
		FORWARD_FUNC(UpdatePendingVisibilityChanges, 0x330);
	}

#undef FORWARD_FUNC

	inline uint16_t GetObjectId()
	{
		return objectId;
	}

	inline uint16_t GetObjectType()
	{
		return objectType;
	}

	inline std::string ToString()
	{
		return fmt::sprintf("[netObj:%d:%s]", objectId, GetTypeString());
	}
};

netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4);
}
