#include "StdInc.h"

#include "Hooking.FlexStruct.h"
#include "Hooking.Patterns.h"

#include "Hooking.Stubs.h"

static uint8_t g_targetEntryPointOffset = 0x0;
static uint8_t g_syncedVehicleOffset = 0x0;

static uint32_t g_modelInfoDataOffset = 0x0;
static uint32_t g_numEntryExitPointsOffset = 0x0;

struct CSyncedVehicle
{
	uint16_t m_VehicleId;
	hook::FlexStruct* m_Vehicle;
};
static_assert(sizeof(CSyncedVehicle) == 0x10, "CSyncedVehicle has wrong size!");

static void (*g_CClonedVehicleFSMInfo_Serialise)(hook::FlexStruct*, hook::FlexStruct*);

static void CClonedVehicleFSMInfo_Serialise(hook::FlexStruct* clonedTaskInfo, hook::FlexStruct* serialiser)
{
	g_CClonedVehicleFSMInfo_Serialise(clonedTaskInfo, serialiser);

    const auto& syncedVehicle = clonedTaskInfo->Get<CSyncedVehicle>(g_syncedVehicleOffset);

	const auto vehicle = syncedVehicle.m_Vehicle;
	if (!vehicle)
		return;

	// NOTE: pretty sure that archetype class offset wont change
	const auto archetype = syncedVehicle.m_Vehicle->Get<hook::FlexStruct*>(0x20);
	const auto modelInfoData = archetype->Get<hook::FlexStruct*>(g_modelInfoDataOffset);
	const auto numEntryExitPoints = modelInfoData->Get<uint8_t>(g_numEntryExitPointsOffset);

	int32_t& targetEntryPoint = clonedTaskInfo->At<int32_t>(g_targetEntryPointOffset);

	targetEntryPoint = std::clamp(targetEntryPoint, 0, numEntryExitPoints - 1);
}

static HookFunction hookFunction([]()
{
	// The game serializes an integer representing the target seat entry/exit point.
	// This serialization is extracted from both CTaskEnterVehicle and CTaskExitVehicle.
	//
	// The target entry point index is later used to resolve entry/exit animation data.
	// If the index is out of range, the lookup returns nullptr.
	//
	// Both CTaskEnterVehicle and CTaskExitVehicle assume the resolved entry/exit point data is valid
	// and dereference it without a nullptr check, which can be exploited by cheaters to cause a crash.

	g_targetEntryPointOffset = *hook::get_pattern<uint8_t>("48 8D 53 ? 48 8B CE FF 50 ? 48 8D 53", 3);
	g_syncedVehicleOffset = *hook::get_pattern<uint8_t>("48 8D 4B ? E8 ? ? ? ? 48 8D 54 24 ? 48 8B CE 44 0F B7 00 48 8B 06 66 44 89 44 24 ? 45 33 C0 FF 90 ? ? ? ? 48 8B 06 45 33 C9 45 8D 41 ? 48 8D 53 ? 48 8B CE FF 50 ? 48 8D 53", 3);

	g_modelInfoDataOffset = *hook::get_pattern<uint32_t>("48 8B 88 ? ? ? ? 38 99 ? ? ? ? 76", 3);
	g_numEntryExitPointsOffset = *hook::get_pattern<uint32_t>("38 99 ? ? ? ? 76 ? 8D 6B", 2);

	g_CClonedVehicleFSMInfo_Serialise = hook::trampoline(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 4F ? E8 ? ? ? ? 48 8D 54 24 ? 48 8D 4F")), CClonedVehicleFSMInfo_Serialise);
});
