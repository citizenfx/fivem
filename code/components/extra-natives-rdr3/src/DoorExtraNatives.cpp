/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <atArray.h>
#include <msgpack.hpp>

#include <Local.h>
#include <Hooking.h>
#include <GameInit.h>
#include <scrEngine.h>

#include <EntitySystem.h>
#include <ConsoleHost.h>
#include <CoreConsole.h>

class DoorSystemEntry
{
public:
	uint32_t doorHash; // 0x0000
	char pad_0004[4]; // 0x0004
	fwEntity* ptrFwEntity; // 0x0008
	DoorSystemEntry* next; // 0x0010
	char pad_0018[124]; // 0x0018
	uint32_t N00000242; // 0x0094
	char pad_0098[4]; // 0x0098
	uint8_t flag; // 0x009C This is door state
	char pad_009D[5]; // 0x009D
}; // Size: 0x00A2

class CDoorsRendered
{
public:
	DoorSystemEntry** entries; // 0x0000
	uint32_t bucketCapacity; // 0x0008
	char pad_000C[4]; // 0x000C
	uint32_t bucketEntries; // 0x0010
}; // Size: 0x0014

struct DoorNativeResult
{
	DoorNativeResult(uint32_t _doorHash, uint32_t _handle)
		: doorHash(_doorHash), handle(_handle)
	{
	}

	uint32_t doorHash;
	uint32_t handle;

	MSGPACK_DEFINE_ARRAY(doorHash, handle)
};

struct DoorDistanceResult
{
	DoorDistanceResult(uint32_t _doorHash, uint32_t _handle, float _distance)
		: doorHash(_doorHash), handle(_handle), distance(_distance)
	{
	}

	uint32_t doorHash;
	uint32_t handle;
	float distance;

	MSGPACK_DEFINE_ARRAY(doorHash, handle, distance)
};

static HookFunction initFunction([]()
{
	static CDoorsRendered* g_doorData = hook::get_address<CDoorsRendered*>(hook::get_pattern("48 8D 0D ? ? ? ? E8 ? ? ? ? 48 85 C0 74 6E"), 3, 7);

	fx::ScriptEngine::RegisterNativeHandler("DOOR_SYSTEM_GET_SIZE", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(g_doorData->bucketEntries);
	});

	fx::ScriptEngine::RegisterNativeHandler("DOOR_SYSTEM_GET_ACTIVE", [](fx::ScriptContext& context)
	{
		std::vector<DoorNativeResult> doorList;

		doorList.reserve(g_doorData->bucketEntries);

		for (int i = 0; i < g_doorData->bucketCapacity; i++)
		{
			DoorSystemEntry* entry = g_doorData->entries[i];

			while (entry != nullptr)
			{
				uint32_t handle = NativeInvoke::Invoke<0xF7424890E4A094C0, uint32_t>(entry->doorHash);
				if (handle != 0 && entry->doorHash != 0)
				{
					doorList.emplace_back(entry->doorHash, handle);
				}
				entry = entry->next;
			}
		}
		context.SetResult(fx::SerializeObject(doorList));
	});

	// GET_CLOSEST_DOOR_HASH - Returns the door hash of the closest door to the given position
	fx::ScriptEngine::RegisterNativeHandler("GET_CLOSEST_DOOR_HASH", [&](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float maxDistance = context.GetArgument<float>(3);

		uint32_t closestDoorHash = 0;
		float closestDistanceSq = maxDistance * maxDistance;

		for (int i = 0; i < g_doorData->bucketCapacity; i++)
		{
			DoorSystemEntry* entry = g_doorData->entries[i];

			while (entry != nullptr)
			{
				if (entry->doorHash != 0 && entry->ptrFwEntity != nullptr)
				{
					uint32_t handle = NativeInvoke::Invoke<0xF7424890E4A094C0, uint32_t>(entry->doorHash);
					if (handle != 0 && entry->doorHash != 0)
					{
						scrVector position = NativeInvoke::Invoke<0xA86D5F069399F44D, scrVector>(handle);

						float dx = position.x - x;
						float dy = position.y - y;
						float dz = position.z - z;
						float distSq = dx * dx + dy * dy + dz * dz;

						if (distSq < closestDistanceSq)
						{
							closestDistanceSq = distSq;
							closestDoorHash = entry->doorHash;
						}
					}
				}
				entry = entry->next;
			}
		}

		context.SetResult<uint32_t>(closestDoorHash);
	});

	// GET_CLOSEST_DOORS - Returns all doors within maxDistance, sorted by distance
	fx::ScriptEngine::RegisterNativeHandler("GET_CLOSEST_DOORS", [&](fx::ScriptContext& context)
	{
		float x = context.GetArgument<float>(0);
		float y = context.GetArgument<float>(1);
		float z = context.GetArgument<float>(2);
		float maxDistance = context.GetArgument<float>(3);
		bool sortByDistance = context.GetArgument<bool>(4);

		float maxDistanceSq = maxDistance * maxDistance;
		std::vector<DoorDistanceResult> doorList;

		for (int i = 0; i < g_doorData->bucketCapacity; i++)
		{
			DoorSystemEntry* entry = g_doorData->entries[i];

			while (entry != nullptr)
			{
				if (entry->doorHash != 0 && entry->ptrFwEntity != nullptr)
				{
					uint32_t handle = NativeInvoke::Invoke<0xF7424890E4A094C0, uint32_t>(entry->doorHash);
					if (handle != 0 && entry->doorHash != 0)
					{
						scrVector position = NativeInvoke::Invoke<0xA86D5F069399F44D, scrVector>(handle);

						float dx = position.x - x;
						float dy = position.y - y;
						float dz = position.z - z;
						float distSq = dx * dx + dy * dy + dz * dz;

						if (distSq <= maxDistanceSq)
						{
							float dist = sqrtf(distSq);
							doorList.emplace_back(entry->doorHash, handle, dist);
						}
					}
				}
				entry = entry->next;
			}
		}

		if (sortByDistance)
		{
			std::sort(doorList.begin(), doorList.end(), [](const DoorDistanceResult& a, const DoorDistanceResult& b)
			{
				return a.distance < b.distance;
			});
		}

		context.SetResult(fx::SerializeObject(doorList));
	});

	// GET_DOOR_HASH_FROM_ENTITY - Returns door hash by door entity handle (reverse of _GET_ENTITY_BY_DOORHASH)
	fx::ScriptEngine::RegisterNativeHandler("GET_DOOR_HASH_FROM_ENTITY", [&](fx::ScriptContext& context)
	{
		int entityHandle = context.GetArgument<int>(0);

		auto* targetEntity = rage::fwScriptGuid::GetBaseFromGuid(entityHandle);
		if (!targetEntity)
		{
			context.SetResult<uint32_t>(0);
			return;
		}

		for (int i = 0; i < g_doorData->bucketCapacity; i++)
		{
			DoorSystemEntry* entry = g_doorData->entries[i];
			while (entry != nullptr)
			{
				if (entry->doorHash != 0 && entry->ptrFwEntity == targetEntity)
				{
					context.SetResult<uint32_t>(entry->doorHash);
					return;
				}
				entry = entry->next;
			}
		}

		context.SetResult<uint32_t>(0);
	});
});
