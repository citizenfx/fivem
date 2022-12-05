#include "StdInc.h"
#include "Hooking.h"

#include <EntitySystem.h>

#include "ScriptWarnings.h"
#include "scrEngine.h"

#include <boost/type_index.hpp>

struct netObject
{
	char pad[10];
	uint16_t objectId;
	char pad2[63];
	bool isRemote;
	char pad3[4];
	fwEntity* gameObject;
};

template<typename T>
static T* getTypedEntity(int guid)
{
	fwEntity* entity = getScriptEntity(guid);

	if (!entity)
	{
		return nullptr;
	}

	if (!entity->IsOfType<T>())
	{
		return nullptr;
	}

	return static_cast<T*>(entity);
}

static hook::cdecl_stub<fwEntity*(int handle)> getScriptEntity([]()
{
	return hook::pattern("44 8B C1 49 8B 41 08 41 C1 F8 08 41 38 0C 00").count(1).get(0).get<void>(-12);
});

static hook::cdecl_stub<void(fwEntity*)> deletePedReal([]()
{
	return hook::get_pattern("48 83 EC 28 48 85 C9 74 12 48 8B D1");
});

struct CPedFactory
{
	virtual ~CPedFactory() = 0;

	CPed* localPlayerPed;
};

static CPedFactory** g_pedFactory;

static void deletePed(fwEntity* entity)
{
	if (*g_pedFactory && (*g_pedFactory)->localPlayerPed == entity)
	{
		return;
	}

	return deletePedReal(entity);
}

static hook::cdecl_stub<void(fwEntity*)> deleteVehicle([]()
{
	return hook::get_pattern("48 8B D9 48 8B 89 D0 00 00 00 48 85 C9 74 05 E8", -17);
});

static hook::cdecl_stub<void(fwEntity*)> deleteObject([]()
{
	return hook::get_pattern("F6 40 70 80 75 0A", -33);
});

static hook::cdecl_stub<void(netObject*, bool)> sendMarkAsNoLongerNeededEvent([]()
{
	return hook::get_pattern("48 8B F9 40 8A EB FF 90 ? ? 00 00 44 8D 7B 01", -34);
});

static hook::cdecl_stub<void(fwEntity*, bool)> markAsNoLongerNeeded([]()
{
	return hook::get_pattern("48 8D 48 08 4C 8B 01 41 FF 50 38", -0x2C);
});

static hook::cdecl_stub<netObject*(uint16_t id)> getNetObjById([]()
{
	return hook::get_call(hook::get_pattern("14 41 0F B7 0E E8 ? ? ? ? 48 8B C8", 5));
});

static fwEntity* GetNetworkObject(void* scriptHandler, int objectId)
{
	auto object = getNetObjById(objectId);

	if (!object)
	{
		fx::scripting::Warningf("entity", __FUNCTION__ ": no object by ID %d\n", objectId);
		return nullptr;
	}

	auto gameObject = object->gameObject;

	if (!gameObject)
	{
		fx::scripting::Warningf("entity", __FUNCTION__ ": no game object for ID %d\n", objectId);
		return nullptr;
	}

	return gameObject;
}

static HookFunction hookFunction([] ()
{
	g_pedFactory = hook::get_address<decltype(g_pedFactory)>(hook::get_pattern("E8 ? ? ? ? 48 8B 05 ? ? ? ? 48 8B 58 08 48 8B CB E8", 8));

	// netObject getters

	// get entity by network ID
	hook::jump(hook::get_pattern("48 FF 62 70 48 83 C4 28", -0x18), GetNetworkObject);

	// get network ID by entity
	rage::scrEngine::NativeHandler getNetID = [](rage::scrNativeCallContext* context)
	{
		auto entityID = context->GetArgument<int>(0);
		auto entity = getScriptEntity(entityID);

		if (!entity)
		{
			fx::scripting::Warningf("entity", "NETWORK_GET_NETWORK_ID_FROM_ENTITY: no such entity (script ID %d)\n", entityID);
			return;
		}

		auto netObject = (::netObject*)entity->GetNetObject();

		if (!netObject)
		{
			// #TODO: string representation of the entity?
			fx::scripting::Warningf("entity", "NETWORK_GET_NETWORK_ID_FROM_ENTITY: no net object for entity (script id %d)\n", entityID);
			return;
		}

		context->SetResult(0, uint64_t(netObject->objectId));
	};

	// NETWORK_GET_NETWORK_ID_FROM_ENTITY
	rage::scrEngine::RegisterNativeHandler(0xA11700682F3AD45C, getNetID);

	// VEH_TO_NET
	rage::scrEngine::RegisterNativeHandler(0xB4C94523F023419C, getNetID);

	// PED_TO_NET
	rage::scrEngine::RegisterNativeHandler(0x0EDEC3C276198689, getNetID);

	// OBJ_TO_NET
	rage::scrEngine::RegisterNativeHandler(0x99BFDC94A603E541, getNetID);

	// NETWORK_DOES_NETWORK_ID_EXIST
	rage::scrEngine::RegisterNativeHandler(0x38CE16C96BD11344, [](rage::scrNativeCallContext* context)
	{
		auto objectId = context->GetArgument<int>(0);
		auto object = getNetObjById(objectId);

		context->SetResult<uint32_t>(0, object != nullptr);
	});

	// NETWORK_DOES_ENTITY_EXIST_WITH_NETWORK_ID
	rage::scrEngine::RegisterNativeHandler(0x18A47D074708FD68, [](rage::scrNativeCallContext* context)
	{
		auto objectId = context->GetArgument<int>(0);
		auto object = getNetObjById(objectId);

		context->SetResult<uint32_t>(0, object && object->gameObject);
	});

	// CGameScriptHandlerObject::GetOwner vs. GetCurrentScriptHandler checks

	// common pattern:
	// call qword ptr [r?x + 38h]
	// mov rbx, rax
	// call GetGameScriptHandler
	// cmp rbx, rax

	// this pattern starts at the + 38h portion, and matches 7 functions, all of which are script calls.
	// these include:
	// - DELETE_ENTITY
	// - DOES_ENTITY_BELONG_TO_THIS_SCRIPT
	// - the common group of:
	//   - SET_ENTITY_AS_NO_LONGER_NEEDED
	//   - SET_PED_AS_NO_LONGER_NEEDED
	//   - SET_VEHICLE_AS_NO_LONGER_NEEDED
	//   - SET_OBJECT_AS_NO_LONGER_NEEDED
	//   (these all use the same function)
	// - DELETE_OBJECT
	// - DELETE_PED
	// - REMOVE_PED_ELEGANTLY

	// in 1290 this is reduced to only two (DELETE_VEHICLE and a leftover of DOES_ENTITY_BELONG_TO_THIS_SCRIPT) - obfuscation at work?
	// 1604 has one one (DELETE_VEHICLE) - we need to reimplement the one left, too?
	{
		auto pattern = hook::pattern("FF 52 38 48 8B D8 E8 ? ? ? ? 48 3B D8").count(1);

		for (int i = 0; i < 1; i++)
		{
			// is this DOES_ENTITY_BELONG_TO_THIS_SCRIPT?
			if (*pattern.get(i).get<uint16_t>(15) == 0xB004) // jnz $+4 (the 04 byte); mov al, 1 (the B0 byte)
			{
				// if so, then skip
				continue;
			}

			hook::nop(pattern.get(i).get<void>(0xE), 2);
		}
	}

	// reimplement the other natives ourselves
	rage::scrEngine::NativeHandler setAsNoLongerNeeded = [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getScriptEntity(*entityRef);

		if (entity)
		{
			if (entity->GetNetObject() && ((netObject*)entity->GetNetObject())->isRemote)
			{
				sendMarkAsNoLongerNeededEvent((netObject*)entity->GetNetObject(), false);
			}
			else
			{
				markAsNoLongerNeeded(entity, true);
			}
		}

		*entityRef = 0;
	};

	// SET_ENTITY_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0xB736A491E64A32CF, setAsNoLongerNeeded);

	// SET_OBJECT_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x3AE22DEB5BA5A3E6, setAsNoLongerNeeded);

	// SET_PED_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x2595DD4236549CE3, setAsNoLongerNeeded);

	// SET_VEHICLE_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x629BFA74418D6239, setAsNoLongerNeeded);

	// DELETE_PED
	rage::scrEngine::RegisterNativeHandler(0x9614299DCB53E54B, [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getTypedEntity<CPed>(*entityRef);

		if (entity)
		{
			deletePed(entity);
		}

		*entityRef = 0;
	});

	// DELETE_OBJECT
	rage::scrEngine::RegisterNativeHandler(0x539E0AE3E6634B9F, [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getTypedEntity<CObject>(*entityRef);

		if (entity)
		{
			// TODO: needs some network checks to verify if local
			deleteObject(entity);
		}

		*entityRef = 0;
	});

	// DELETE_ENTITY
	rage::scrEngine::RegisterNativeHandler(0xAE3CBE5BF394C9C9, [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getScriptEntity(*entityRef);

		if (entity)
		{
			switch (entity->GetType())
			{
			case 3:
				deleteVehicle(entity);
				break;
			case 4:
				deletePed(entity);
				break;
			case 5:
				deleteObject(entity);
				break;
			}
		}

		*entityRef = 0;
	});
});
