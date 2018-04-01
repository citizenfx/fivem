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

public:
	bool CanApplyToObject(netObject* object);

	bool ReadFromBuffer(int flags, int flags2, rage::netBuffer* buffer, void* netLogStub);

public:
	static netSyncTree* GetForType(NetObjEntityType type);
};
}
