#include "StdInc.h"
#include "Hooking.h"

#include "scrEngine.h"

#include <boost/type_index.hpp>

class fwEntity;

struct netObject
{
	char pad[64]; // +0
	uint16_t objectType; // +64
	uint16_t objectId; // +66
	char pad2[3]; // +68 
	bool isRemote; // +71
	char pad3[16]; // +72
	fwEntity* gameObject; // +88
};

class fwEntity
{
public:
	virtual ~fwEntity() = 0;

	char pad[40]; // +8
	uint8_t entityType; // +48
	char pad2[175]; // +49
	netObject* netObject; // +224
};

class CPickup : public fwEntity
{

};

class CObject : public fwEntity
{

};

class CVehicle : public fwEntity
{

};

class CPed : public fwEntity
{

};

template<typename T>
static T* getTypedEntity(int guid)
{
	fwEntity* entity = getScriptEntity(guid);

	if (!entity)
	{
		return nullptr;
	}

	return static_cast<T*>(entity);
}

static hook::cdecl_stub<uint32_t(fwEntity*)> getScriptGuidForEntity([]()
{
	return hook::pattern("32 DB E8 ? ? ? ? 48 85 C0 75 ? 8A 05").count(1).get(0).get<void>(-35);
});

static hook::cdecl_stub<fwEntity*(int handle)> getScriptEntity([]()
{
	return hook::pattern("45 8B C1 41 C1 F8 08 45 38 0C 00 75 ? 8B 42 ? 41 0F AF C0").count(1).get(0).get<void>(-81);
});

static hook::cdecl_stub<void(fwEntity*)> deletePed([]()
{
	return hook::get_pattern("84 D2 74 ? 48 8B 01 48 83 C4 28 48 FF A0", -9);
});

static hook::cdecl_stub<void(fwEntity*)> deleteVehicle([]()
{
	return hook::get_pattern("48 8B D9 45 84 C0 0F 84 ? ? ? ? E8", -37);
});

static hook::cdecl_stub<void(fwEntity*)> deleteObject([]()
{
	return hook::get_pattern("40 8A 78 ? C1 E9 1A 83 E1 1F 83 F9 03", -67);
});

static hook::cdecl_stub<void(netObject*, bool)> sendMarkAsNoLongerNeededEvent([]()
{
	return hook::get_pattern("49 8B 04 C0 40 84 34 01 0F 84", -47);
});

static hook::cdecl_stub<void(fwEntity*, bool)> markAsNoLongerNeeded([]()
{
	return hook::get_pattern("48 83 C1 10 E8 ? ? ? ? 48 8B D8 EB", -37);
});

static hook::cdecl_stub<netObject* (uint16_t id)> getNetObjById([]()
{
	return hook::get_call(hook::get_pattern("48 8B F8 8A 46 ? 3C 03 75", -13));
});

static HookFunction hookFunction([]()
{
	// get network ID by entity
	rage::scrEngine::NativeHandler getNetID = [](rage::scrNativeCallContext* context)
	{
		auto entity = getScriptEntity(context->GetArgument<int>(0));

		if (!entity)
		{
			trace("NETWORK_GET_NETWORK_ID_FROM_ENTITY: no such entity\n");
			return;
		}

		auto netObject = entity->netObject;

		if (!netObject)
		{
			trace("NETWORK_GET_NETWORK_ID_FROM_ENTITY: no net object for entity\n");
			return;
		}

		context->SetResult(0, uint64_t(netObject->objectId));
	};

	// get entity by network ID
	rage::scrEngine::NativeHandler getNetObj = [](rage::scrNativeCallContext* context)
	{
		auto objectId = context->GetArgument<int>(0);
		auto object = getNetObjById(objectId);

		if (!object)
		{
			trace("NETWORK_GET_ENTITY_FROM_NETWORK_ID: no object by ID %d\n", objectId);
			return;
		}

		auto gameObject = object->gameObject;

		if (!gameObject)
		{
			trace("NETWORK_GET_ENTITY_FROM_NETWORK_ID: no game object for ID %d\n", objectId);
			return;
		}

		auto entityHandle = getScriptGuidForEntity(gameObject);
		context->SetResult(0, entityHandle);
	};

	// NETWORK_GET_NETWORK_ID_FROM_ENTITY
	rage::scrEngine::RegisterNativeHandler(0xA11700682F3AD45C, getNetID);

	// VEH_TO_NET
	rage::scrEngine::RegisterNativeHandler(0xB4C94523F023419C, getNetID);

	// PED_TO_NET
	rage::scrEngine::RegisterNativeHandler(0x0EDEC3C276198689, getNetID);

	// OBJ_TO_NET
	rage::scrEngine::RegisterNativeHandler(0x99BFDC94A603E541, getNetID);

	// NETWORK_GET_ENTITY_FROM_NETWORK_ID
	rage::scrEngine::RegisterNativeHandler(0xCE4E5D9B0A4FF560, getNetObj);

	// NET_TO_ENT
	rage::scrEngine::RegisterNativeHandler(0xBFFEAB45A9A9094A, getNetObj);

	// NET_TO_VEH
	rage::scrEngine::RegisterNativeHandler(0x367B936610BA360C, getNetObj);

	// NET_TO_PED
	rage::scrEngine::RegisterNativeHandler(0xBDCD95FC216A8B3E, getNetObj);

	// NET_TO_OBJ
	rage::scrEngine::RegisterNativeHandler(0xD8515F5FEA14CB3F, getNetObj);

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
	{
		auto pattern = hook::pattern("38 48 8B D8 E8 ? ? ? ? 48 3B D8").count(13);

		for (int i = 0; i < 13; i++)
		{
			// is this DOES_ENTITY_BELONG_TO_THIS_SCRIPT?
			if (*pattern.get(i).get<uint16_t>(13) == 0xB004) // jnz $+4 (the 04 byte); mov al, 1 (the B0 byte)
			{
				// if so, then skip
				continue;
			}

			hook::nop(pattern.get(i).get<void>(0xC), 2);
		}
	}

	// reimplement the other natives ourselves
	rage::scrEngine::NativeHandler setAsNoLongerNeeded = [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getScriptEntity(*entityRef);

		if (entity)
		{
			if (entity->netObject && entity->netObject->isRemote)
			{
				sendMarkAsNoLongerNeededEvent(entity->netObject, false);
			}
			else
			{
				markAsNoLongerNeeded(entity, true);
			}
		}

		*entityRef = 0;
	};

	// SET_ENTITY_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x4971D2F8162B9674, setAsNoLongerNeeded);

	// SET_OBJECT_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x3AE22DEB5BA5A3E6, setAsNoLongerNeeded);

	// SET_PED_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x2595DD4236549CE3, setAsNoLongerNeeded);

	// SET_VEHICLE_AS_NO_LONGER_NEEDED
	rage::scrEngine::RegisterNativeHandler(0x629BFA74418D6239, setAsNoLongerNeeded);

	// DELETE_PED
	rage::scrEngine::RegisterNativeHandler(0xCC0EF140F99365C5, [](rage::scrNativeCallContext* context)
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
	rage::scrEngine::RegisterNativeHandler(0x931914268722C263, [](rage::scrNativeCallContext* context)
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
	rage::scrEngine::RegisterNativeHandler(0x4CD38C78BD19A497, [](rage::scrNativeCallContext* context)
	{
		auto entityRef = context->GetArgument<int*>(0);
		auto entity = getScriptEntity(*entityRef);

		if (entity)
		{
			switch (entity->entityType)
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
