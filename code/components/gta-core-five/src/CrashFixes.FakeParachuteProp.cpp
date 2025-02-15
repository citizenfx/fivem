#include <StdInc.h>
#include "CrashFixes.FakeParachuteProp.h"

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <Hooking.FlexStruct.h>

#include <jitasm.h>
#include <GameInit.h>
#include <unordered_set>

#include "CrossBuildRuntime.h"

uint32_t taskEntityOffset = 0;
uint32_t dynamicEntityComponentOffset = 0;
uint32_t animDirectorOffset = 0;
std::once_flag traceOnceFlag;

uint32_t parachuteObjectOffset = 0;
uint32_t drawHandlerOffset = 0;

uint32_t taskTakeOffPedVariationPropOffset = 0;

std::unordered_set<uint32_t> g_ParachuteModelWhitelist;
std::unordered_set<uint32_t> g_ParachutePackModelWhitelist;

template<bool UseTaskPropEntity>
bool IsTaskFSMEntityAnimDirectorValid(hook::FlexStruct* self)
{
	hook::FlexStruct* taskEntity = self->Get<hook::FlexStruct*>(UseTaskPropEntity ? taskTakeOffPedVariationPropOffset : taskEntityOffset);

	// We only care about validating a entity's animDirector if the entity exists already
	// as entity creation might be deferred for tasks such as CTaskParachuteObject
	if (taskEntity)
	{
		hook::FlexStruct* dynamicEntityComponent = taskEntity->Get<hook::FlexStruct*>(dynamicEntityComponentOffset);

		if (!dynamicEntityComponent)
		{
			return false;
		}

		hook::FlexStruct* animDirector = dynamicEntityComponent->Get<hook::FlexStruct*>(animDirectorOffset);

		if (!animDirector)
		{
			std::call_once(traceOnceFlag, []()
			{
				trace("Fake parachute model crash prevented [CFX-1907]\n");
			});

			return false;
		}
	}

	return true;
}

uint64_t (*g_CTaskParachuteObject_UpdateFSM)(hook::FlexStruct* self, int state, int subState);
uint64_t CTaskParachuteObject_UpdateFSM(hook::FlexStruct* self, int state, int subState)
{
	if (!IsTaskFSMEntityAnimDirectorValid<false>(self))
	{
		return 0;
	}

	return g_CTaskParachuteObject_UpdateFSM(self, state, subState);
}

uint64_t (*g_CTaskTakeOffPedVariation_UpdateFSM)(hook::FlexStruct* self, int state, int subState);
uint64_t CTaskTakeOffPedVariation_UpdateFSM(hook::FlexStruct* self, int state, int subState)
{
	if (!IsTaskFSMEntityAnimDirectorValid<true>(self))
	{
		return 0;
	}

	return g_CTaskTakeOffPedVariation_UpdateFSM(self, state, subState);
}

uint64_t (*g_CTaskTakeOffPedVariation_AttachProp)(hook::FlexStruct* self);
uint64_t CTaskTakeOffPedVariation_AttachProp(hook::FlexStruct* self)
{
	if (!IsTaskFSMEntityAnimDirectorValid<true>(self))
	{
		return 0;
	}

	return g_CTaskTakeOffPedVariation_AttachProp(self);
}

void (*g_CTaskParachute_SetParachuteTintIndex)(hook::FlexStruct* self);
void CTaskParachute_SetParachuteTintIndex(hook::FlexStruct* self)
{
	hook::FlexStruct* parachuteObject = self->Get<hook::FlexStruct*>(parachuteObjectOffset);
	if (!parachuteObject)
	{
		return;
	}

	hook::FlexStruct* drawHandler = parachuteObject->Get<hook::FlexStruct*>(drawHandlerOffset);
	if (!drawHandler)
	{
		return;
	}

	g_CTaskParachute_SetParachuteTintIndex(self);
}

bool IsParachuteModelAuthorized(const uint32_t& modelNameHash)
{
	return g_ParachuteModelWhitelist.find(modelNameHash) != g_ParachuteModelWhitelist.end();
}

bool IsParachutePackModelAuthorized(const uint32_t& modelNameHash)
{
	return g_ParachutePackModelWhitelist.find(modelNameHash) != g_ParachutePackModelWhitelist.end();
}

void AddAuthorizedParachuteModel(const uint32_t& modelNameHash)
{
	if (modelNameHash == 0 || IsParachuteModelAuthorized(modelNameHash))
	{
		return;
	}

	g_ParachuteModelWhitelist.insert(modelNameHash);
}

void AddAuthorizedParachutePackModel(const uint32_t& modelNameHash)
{
	if (modelNameHash == 0 || IsParachutePackModelAuthorized(modelNameHash))
	{
		return;
	}

	g_ParachutePackModelWhitelist.insert(modelNameHash);
}

struct fwEntity;

struct netObject
{
	char pad[10];
	uint16_t objectId;
	char pad2[63];
	bool isRemote;
	char pad3[4];
	fwEntity* gameObject;
};

struct fwArchetype
{
	char vtbl[8];
	char pad[16];
	uint32_t hash;
};

struct fwEntity
{
	char vtbl[8];
	char m_pad[8];
	char m_extensionList[8];
	char m_pad2[8];
	fwArchetype* m_archetype;
};

static hook::cdecl_stub<netObject*(uint16_t id)> getNetObjById([]()
{
	return hook::get_call(hook::get_pattern("14 41 0F B7 0E E8 ? ? ? ? 48 8B C8", 5));
});

static hook::cdecl_stub<void(void* syncEntity, uint16_t* entityId)> setEntityId([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? EB ? 48 8B 07 48 8D 56"));
});

void ResetWhitelists()
{
	g_ParachuteModelWhitelist = {
		HashString("prop_parachute"),
		HashString("prop_v_parachute"),
		HashString("p_parachute1_mp_dec"),
		HashString("p_parachute1_mp_s"),
		HashString("p_parachute1_s"),
		HashString("p_parachute1_sp_dec"),
		HashString("p_parachute1_sp_s"),
		HashString("cj_parachute")
	};

	g_ParachutePackModelWhitelist = {
		HashString("p_parachute_s")
	};
}

void SanitizeParachuteNetEntity(void* syncEntity, uint16_t* entityId)
{
	uint16_t zero = 0;

	netObject* netObj = getNetObjById(*entityId);
	if (netObj == nullptr || netObj->gameObject == nullptr || netObj->gameObject->m_archetype == nullptr)
	{
		setEntityId(syncEntity, &zero);
		return;
	}

	fwEntity* entity = netObj->gameObject;
	IsParachuteModelAuthorized(entity->m_archetype->hash) ? setEntityId(syncEntity, entityId) : setEntityId(syncEntity, &zero);
}

static HookFunction hookFunction([]
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		return;
	}

	g_CTaskParachuteObject_UpdateFSM = hook::trampoline(hook::get_pattern<void>("48 83 EC ? 85 D2 0F 88 ? ? ? ? 75 ? 45 85 C0 75 ? 48 8B 41"), &CTaskParachuteObject_UpdateFSM);
	g_CTaskParachute_SetParachuteTintIndex = hook::trampoline(hook::get_pattern<void>("48 89 5C 24 ? 57 48 83 EC ? 48 8B 81 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B 40"), &CTaskParachute_SetParachuteTintIndex);

	taskEntityOffset = *hook::get_pattern<uint8_t>("48 8B 5E ? 4C 8D 05", 3);
	dynamicEntityComponentOffset = *hook::get_pattern<uint8_t>("48 8B 43 ? 48 85 C0 74 ? 48 8B 78 ? 48 8B CF E8 ? ? ? ? 48 8B D5", 3);
	animDirectorOffset = *hook::get_pattern<uint8_t>("48 8B 78 ? 48 8B CF E8 ? ? ? ? 48 8B D5", 3);

	parachuteObjectOffset = *hook::get_pattern<uint32_t>("48 8B 81 ? ? ? ? 48 8B D9 48 85 C0 74 ? 48 8B 40 ? 48 8B 78", 3);
	drawHandlerOffset = *hook::get_pattern<uint8_t>("48 8B 40 ? 48 8B 78", 3);

	taskTakeOffPedVariationPropOffset = *hook::get_pattern<uint32_t>("48 8B 81 ? ? ? ? 33 C9 48 85 C0 74 ? 48 8B 40 ? 48 85 C0", 3);

	g_CTaskTakeOffPedVariation_UpdateFSM = hook::trampoline(hook::get_pattern<void>("48 83 EC ? 4C 8B C9 85 D2 78 ? B8"), &CTaskTakeOffPedVariation_UpdateFSM);
	g_CTaskTakeOffPedVariation_AttachProp = hook::trampoline(hook::get_pattern<void>("48 89 5C 24 ? 57 48 83 EC ? 0F 29 74 24 ? 48 8B D9 E8 ? ? ? ? 84 C0"), &CTaskTakeOffPedVariation_AttachProp);

	// Sanitize the parachute object's model when it's replicated to a client.
	{
		auto location = hook::get_pattern("E8 ? ? ? ? 8A 46 ? 45 33 C9");
		hook::call(location, SanitizeParachuteNetEntity);
	}

	// Initialize the whitelists and make sure they get reset on disconnect
	{
		ResetWhitelists();

		OnKillNetworkDone.Connect([]()
		{
			ResetWhitelists();
		});
	}
});
