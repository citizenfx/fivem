/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include "NativeWrappers.h"

#include <ScriptEngine.h>
#include <ScriptSerialization.h>

#include <atArray.h>
#include <msgpack.hpp>

#include <Local.h>
#include <Hooking.h>
#include <GameInit.h>

static struct DoorNativeResult
{
	uint32_t system_id;
	uint32_t handle;

	MSGPACK_DEFINE_ARRAY(system_id, handle)
};

typedef struct DoorSystemEntry
{
	fwEntity* door;
	DoorSystemEntry* next;
} DoorSystemEntry;

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::get_pattern("48 F7 F9 49 8B 48 08 48 63 D0 C1 E0 08 0F B6 1C 11 03 D8", -0x68);
});

static HookFunction initFunction([]()
{
	// All registered doors;
	static atArray<void*>* g_doorData = hook::get_address<atArray<void*>*>(hook::get_pattern("0F 28 DA 45 33 D2 33 C0 45 33 C0", -0x4));

	// Each CDoor on instantiation adds itself to a global list of all active door objects.
	static DoorSystemEntry** g_doorsystem_objs = hook::get_address<DoorSystemEntry**>(hook::get_pattern("0F 28 92 90 00 00 00 F3 41 0F 10 19 F3 41 0F 10", -0x22));

	// CDoor + X = CDoorSystemData
	static int doorSystemOffset = *hook::get_pattern<int32_t>("48 89 5C 24 08 57 48 83 EC 20 48 8B 81 90 00 00 00", 0x54 + 0x3);

	// CDoorSystemData + X = Identifier
	// TODO: Change this pattern.
	static int systemIDOffset = *hook::get_pattern<int32_t>("4C 89 59 08 44 89 59 18 66 44 89 59 1C", 0x63 + 0x2);

	fx::ScriptEngine::RegisterNativeHandler("DOOR_SYSTEM_GET_SIZE", [](fx::ScriptContext& context)
	{
		context.SetResult<int>(g_doorData->GetSize());
	});

	fx::ScriptEngine::RegisterNativeHandler("DOOR_SYSTEM_GET_ACTIVE", [](fx::ScriptContext& context)
	{
		std::vector<DoorNativeResult> doorList;

		DoorSystemEntry* entry = *g_doorsystem_objs;
		while (entry != nullptr)
		{
			if (entry->door != nullptr)
			{
				char* doorSystem = *(char**)((char*)entry->door + doorSystemOffset);
				if (doorSystem != nullptr)
				{
					uint32_t handle = getScriptGuidForEntity(entry->door);
					uint32_t systemID = *reinterpret_cast<uint32_t*>(doorSystem + systemIDOffset);
					if (handle != 0 && systemID != 0)
					{
						doorList.push_back({ systemID, handle });
					}
				}
			}

			entry = entry->next;
		}

		context.SetResult(fx::SerializeObject(doorList));
	});
});
