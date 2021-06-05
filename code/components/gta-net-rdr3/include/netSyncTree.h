#pragma once

enum class NetObjEntityType
{
	Animal = 0,
	Automobile = 1,
	Bike = 2,
	Boat = 3,
	Door = 4,
	Heli = 5,
	Object = 6,
	Ped = 7,
	Pickup = 8,
	PickupPlacement = 9,
	Plane = 10,
	Submarine = 11,
	Player = 12,
	Trailer = 13,
	Train = 14,
	DraftVeh = 15,
	StatsTracker = 16,
	PropSet = 17,
	AnimScene = 18,
	GroupScenario = 19,
	Herd = 20,
	Horse = 21,
	WorldState = 22,
	WorldProjectile = 23,
	Incident = 24,
	Guardzone = 25,
	PedGroup = 26,
	CombatDirector = 27,
	PedSharedTargeting = 28,
	Persistent = 29,
	Max = 30,
};

const char* GetNetObjEntityName(uint16_t type);

namespace rage
{
class netObject;
class datBitBuffer;
class netSyncNodeBase;

class netSyncTree
{
public:
	virtual ~netSyncTree() = 0;

	virtual void* InitialiseTree() = 0;

	virtual void Write(uint32_t flags, uint32_t objFlags, netObject* object, datBitBuffer* buffer, uint64_t time, uint8_t targetPlayer, uint64_t* outNull, void* null) = 0;

	virtual bool m_18(netObject* object, uint32_t unk) = 0;

	virtual void ApplyToObject(netObject* object, void*) = 0;

	virtual void m_10() = 0;

	virtual void m_20(uint32_t, uint32_t, rage::datBitBuffer*, void*) = 0;

	virtual void m_28() = 0;

	virtual void m_30() = 0;

	virtual void m_38() = 0;

	virtual void m_40() = 0;

	virtual void m_48() = 0;

	virtual void m_50() = 0;

	virtual void m_58() = 0;

	virtual void m_60(rage::netObject* object, void*, uint8_t) = 0;

	virtual void m_68() = 0;

	virtual void m_70() = 0;

	virtual void m_78() = 0;

	virtual void m_80() = 0;

	virtual void m_88() = 0;

	virtual void m_90() = 0;

	virtual void m_98() = 0;

	virtual void m_A0() = 0;

	virtual void m_A8() = 0;

	virtual void m_B0() = 0;

	virtual void m_B8() = 0;

	virtual bool m_C0(int) = 0;

	virtual void m_C8() = 0;

	virtual void m_D0() = 0;

	virtual void m_D8() = 0;

public:
	bool CanApplyToObject(netObject* object);

	bool ReadFromBuffer(int flags, int flags2, rage::datBitBuffer* buffer, void* netLogStub);

	bool WriteTreeCfx(int flags, int objFlags, netObject* object, datBitBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull, uint32_t* lastChangeTime);

	void AckCfx(netObject* object, uint32_t timestamp);

	/*inline void WriteTreeCfx(int flags, int objFlags, netObject* object, netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull)
	{
		WriteTree(flags, objFlags, object, buffer, time, logger, targetPlayer, outNull);
	}*/

public:
	static netSyncTree* GetForType(NetObjEntityType type);

private:
	char pad[168]; // +8

public:
	rage::netSyncNodeBase* syncNode; // +176
};
}
