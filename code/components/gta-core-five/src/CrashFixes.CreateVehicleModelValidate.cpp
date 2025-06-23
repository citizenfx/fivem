#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <CoreConsole.h>

#include "CrossBuildRuntime.h"

static uint32_t g_dataNodeModelHashOffset = 0;
static uint32_t g_modelInfoModelTypeOffset = 0;
constexpr uint8_t kModelTypeVehicle = 5;

struct bitField
{
	uint8_t m_type : 5;
	uint8_t m_bHasPreRenderEffects : 1;
	uint8_t m_bHasDrawableProxyForWaterReflections : 1;
	uint8_t m_tbdGeneralFlag : 1;
};

static hook::FlexStruct* (*fwArchetypeManager_GetArchetypeFromHashKey)(const uint32_t modelHash, uint32_t* modelId);

static void (*CNetObjVehicle_SetVehicleCreateData_Orig)(hook::FlexStruct* self, hook::FlexStruct* vehicleCreateData);
static void CNetObjVehicle_SetVehicleCreateData(hook::FlexStruct* self, hook::FlexStruct* vehicleCreateData)
{
	const uint32_t modelHash = vehicleCreateData->Get<uint32_t>(g_dataNodeModelHashOffset);
	hook::FlexStruct* modelInfo = fwArchetypeManager_GetArchetypeFromHashKey(modelHash, nullptr);
	if (!modelInfo)
	{
		return;
	}

	const bitField modelType = modelInfo->Get<bitField>(g_modelInfoModelTypeOffset);
	if (modelType.m_type != kModelTypeVehicle)
	{
		trace("CNetObjVehicle_SetVehicleCreateData: model type %d/hash %x is not a vehicle, skipping.\n", modelType.m_type, modelHash);
		return;
	}

	CNetObjVehicle_SetVehicleCreateData_Orig(self, vehicleCreateData);
}

static HookFunction hookFunction([]
{
	CNetObjVehicle_SetVehicleCreateData_Orig = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 55 56 57 41 56 41 57 48 8D 68 ? 48 81 EC ? ? ? ? 0F 29 70 ? 8B 45"), CNetObjVehicle_SetVehicleCreateData);
	g_dataNodeModelHashOffset = *hook::get_pattern<uint32_t>("8B 8E ? ? ? ? 0B C7", 2);

	fwArchetypeManager_GetArchetypeFromHashKey = reinterpret_cast<decltype(fwArchetypeManager_GetArchetypeFromHashKey)>(hook::get_call(hook::get_pattern("E8 ? ? ? ? 0F B7 45 ? 66 89 45 ? 8B 45 ? 48 8D 4D ? 0B C7")));
	g_modelInfoModelTypeOffset = *hook::get_pattern<uint32_t>("8A 86 ? ? ? ? 24 ? 3C ? 75 ? 48 8D 4D", 2);
});
