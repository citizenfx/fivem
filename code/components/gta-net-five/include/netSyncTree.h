#pragma once

#include <CrossBuildRuntime.h>

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

inline int MapNetSyncTreeMethod(int offset)
{
	if (offset >= 0xD0 && xbr::IsGameBuildOrGreater<2545>())
	{
		offset += 0x10;
	}

	return offset;
}

namespace rage
{
class netObject;
class datBitBuffer;
class netSyncNodeBase;

class netSyncTree
{
public:
	virtual ~netSyncTree() = 0;

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
	using TFn = decltype(&netSyncTree::name); \
	void** vtbl = *(void***)(this);         \
	return (this->*(get_member<TFn>(vtbl[MapNetSyncTreeMethod(offset) / 8])))(__VA_ARGS__);

	inline void WriteTree(int flags, int objFlags, netObject* object, datBitBuffer* buffer, uint32_t time, void* logger, uint8_t targetPlayer, void* outNull)
	{
		FORWARD_FUNC(WriteTree, 0x8, flags, objFlags, object, buffer, time, logger, targetPlayer, outNull);
	}

	inline void ApplyToObject(netObject* object, void* unk)
	{
		FORWARD_FUNC(ApplyToObject, 0x10, object, unk);
	}

	inline void* GetCreationDataNode()
	{
		FORWARD_FUNC(GetCreationDataNode, 0x50);
	}

	inline bool m_D0(int unk)
	{
		FORWARD_FUNC(m_D0, 0xD0, unk);
	}

#undef FORWARD_FUNC

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
	char pad[8]; // +8

public:
	rage::netSyncNodeBase* syncNode; // +16
};
}
