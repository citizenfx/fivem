/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include "EntitySystem.h"

#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <atArray.h>
#include <msgpack.hpp>

#include <Local.h>
#include <Hooking.h>
#include <GameInit.h>
#include <scrEngine.h>

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
});
