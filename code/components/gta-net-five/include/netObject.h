#pragma once

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
	uint16_t objectType;
	uint16_t objectId;
	uint32_t pad;

	CNetworkSyncDataULBase syncData;

	inline netBlender* GetBlender()
	{
		return *(netBlender**)((uintptr_t)this + 88);
	}

	virtual ~netObject() = 0;

	virtual void m_8() = 0;
	virtual void m_10() = 0;
	virtual void m_18() = 0;
	virtual void* m_20() = 0;
	virtual void m_28() = 0;
	virtual netSyncTree* GetSyncTree() = 0;
	virtual void m_38() = 0;
	virtual void m_40() = 0;
	virtual void m_48() = 0;
	virtual void m_50() = 0;
	virtual void m_58() = 0;
	virtual void m_60() = 0;
	virtual void m_68() = 0;
	virtual void m_70() = 0;
	virtual void m_78() = 0;
	virtual void* GetGameObject() = 0;
	virtual void m_88() = 0;
	virtual void m_90() = 0;
	virtual void m_98() = 0;
	virtual int GetObjectFlags() = 0;
	virtual void m_A8() = 0;
	virtual void m_B0() = 0;
	virtual void m_B8() = 0;
	virtual void m_C0() = 0;
	virtual void m_C8() = 0;
	virtual int GetSyncFrequency() = 0;
	virtual void m_D8() = 0;
	virtual void m_E0() = 0;
	virtual void m_E8() = 0;
	virtual void m_F0() = 0;
	virtual void m_F8() = 0;
	virtual void Update() = 0;
	virtual void m_108() = 0;
	virtual void m_110() = 0;
	virtual void m_118() = 0;
	virtual void m_120() = 0;
	virtual void m_128() = 0;
	virtual void m_130() = 0;
	virtual void m_138() = 0;
	virtual void m_140() = 0;
	virtual void m_148() = 0;
	virtual void m_150() = 0;
	virtual bool m_158(void* player, int type, int* outReason) = 0;
	virtual void m_160() = 0;
	virtual bool m_168(int* outReason) = 0;
	virtual void m_170() = 0;
	virtual void m_178() = 0;
	virtual void m_180() = 0;
	virtual void m_188() = 0;
	virtual void m_190() = 0;
	virtual void m_198() = 0;
	virtual void m_1A0() = 0;
	virtual void m_1A8() = 0;
	virtual void m_1B0() = 0;
	virtual void m_1B8() = 0;
	virtual void m_1C0() = 0;
	virtual void m_1C8() = 0;
	virtual void m_1D0() = 0;
	virtual void m_1D8() = 0;
	virtual void m_1E0() = 0;
};

netObject* CreateCloneObject(NetObjEntityType type, uint16_t objectId, uint8_t a2, int a3, int a4);
}


class CNetGamePlayer
{
public:
	virtual ~CNetGamePlayer() = 0;

public:
	uint8_t pad[8];
	void* nonPhysicalPlayerData;
	uint8_t pad2[20];
	uint8_t activePlayerIndex;
	uint8_t physicalPlayerIndex;
	uint8_t pad3[2];
	uint8_t pad4[120];
	void* playerInfo;
};
