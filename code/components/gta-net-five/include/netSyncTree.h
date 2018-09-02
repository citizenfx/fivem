#pragma once

enum class NetObjEntityType
{
	Automobile = 0,
	Bike = 1,
	Boat = 2,
	Door = 3,
	Heli = 4,
	Object = 5,
	Ped = 6,
	Pickup = 7,
	PickupPlacement = 8,
	Plane = 9,
	Submarine = 10,
	Player = 11,
	Trailer = 12,
	Train = 13,
	Max = 14
};

namespace rage
{
class netObject;
class netBuffer;

class netSyncTree
{
public:
	virtual ~netSyncTree() = 0;

	virtual void WriteTree(int flags, int objFlags, netObject* object, netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull) = 0;

	virtual void ApplyToObject(netObject* object, void*) = 0;

	virtual void m_18() = 0;

	virtual void m_20(uint32_t, uint32_t, rage::netBuffer*, void*) = 0;

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

	virtual void m_C0() = 0;

	virtual void m_C8() = 0;

	virtual bool m_D0(int) = 0;

	virtual void m_D8() = 0;

public:
	bool CanApplyToObject(netObject* object);

	bool ReadFromBuffer(int flags, int flags2, rage::netBuffer* buffer, void* netLogStub);

	bool WriteTreeCfx(int flags, int objFlags, netObject* object, netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull);

	void AckCfx(netObject* object, uint32_t timestamp);

	/*inline void WriteTreeCfx(int flags, int objFlags, netObject* object, netBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull)
	{
		WriteTree(flags, objFlags, object, buffer, time, logger, targetPlayer, outNull);
	}*/

public:
	static netSyncTree* GetForType(NetObjEntityType type);
};
}
