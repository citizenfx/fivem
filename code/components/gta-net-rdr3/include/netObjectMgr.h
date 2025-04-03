#pragma once

#include <netObject.h>
#include <CrossBuildRuntime.h>
#include <rlNetBuffer.h>

class CNetGamePlayer;

namespace rage
{
class netObjectMgr
{
public:
	virtual ~netObjectMgr() = 0;

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

#define FORWARD_FUNC(name, offset, ...)    \
	using TFn = decltype(&netObjectMgr::name); \
	void** vtbl = *(void***)(this);        \
	return (this->*(get_member<TFn>(vtbl[(offset / 8) + ((offset > 0x68) ? (xbr::IsGameBuildOrGreater<1436>() ? 1 : 0) : 0)])))(__VA_ARGS__);

public:
	inline void UnregisterNetworkObject(rage::netObject* object, int reason, bool force1, bool force2)
	{
		FORWARD_FUNC(UnregisterNetworkObject, 0x38, object, reason, force1, force2);
	}

	inline void ChangeOwner(rage::netObject* object, CNetGamePlayer* player, int migrationType)
	{
		FORWARD_FUNC(ChangeOwner, 0x40, object, player, migrationType);
	}

	inline void RegisterNetworkObject(rage::netObject* entity)
	{
		// in 1436 R* added 1 more method right before RegisterNetworkObject
		FORWARD_FUNC(RegisterNetworkObject, 0x70, entity);
	}

private:
	struct atDNetObjectNode
	{
		virtual ~atDNetObjectNode();

		netObject* object;
		atDNetObjectNode* next;
	};

	struct ObjectHolder
	{
		atDNetObjectNode* objects;
		netObject** unk; // might not just be a netObject**
	};

private:
	ObjectHolder m_objects[32];

public:
	template<typename T>
	inline void ForAllNetObjects(int playerId, const T& callback)
	{
		for (auto node = m_objects[playerId].objects; node; node = node->next)
		{
			if (node->object)
			{
				callback(node->object);
			}
		}
	}

	netObject* GetNetworkObject(uint16_t id, bool a3);

	static netObjectMgr* GetInstance();
};

enum eSyncDataSerializer : uint8_t
{
	SYNC_DATA_UNKNOWN = 0x0,
	SYNC_DATA_READER = 0x1,
	SYNC_DATA_WRITER = 0x2,
	SYNC_DATA_CLEANER = 0x3,
	SYNC_DATA_UNK = 0x4,
};

struct CSyncDataBase
{
	char m_pad1[8];
	eSyncDataSerializer m_type;
	char m_pad[15];
};

struct CSyncDataReader : CSyncDataBase
{
	datBitBuffer* m_buffer;
};

struct CSyncDataWriter : CSyncDataBase
{
	datBitBuffer* m_buffer;
};

struct CSyncDataSizeCalculator : CSyncDataBase
{
	uint32_t m_size;
};
}
