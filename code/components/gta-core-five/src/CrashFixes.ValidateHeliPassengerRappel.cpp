#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

// NOTE: took from CrashFixes.ValidateSubmarineTasks.cpp
enum eVehicleType : uint32_t
{
	VEHICLE_TYPE_CAR = 0,
	VEHICLE_TYPE_PLANE,
	VEHICLE_TYPE_TRAILER,
	VEHICLE_TYPE_QUADBIKE,
	VEHICLE_TYPE_DRAFT,
	VEHICLE_TYPE_SUBMARINECAR,
	VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE,
	VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE,
	VEHICLE_TYPE_HELI,
	VEHICLE_TYPE_BLIMP,
	VEHICLE_TYPE_AUTOGYRO,
	VEHICLE_TYPE_BIKE,
	VEHICLE_TYPE_BICYCLE,
	VEHICLE_TYPE_BOAT,
	VEHICLE_TYPE_TRAIN,
	VEHICLE_TYPE_SUBMARINE,
};

static int32_t g_VehicleTypeOffset;
static int32_t g_PedVehicleOffset;

static bool (*g_CTaskHeliPassengerRappel_ProcessPostMovement)(hook::FlexStruct*);

static bool CTaskHeliPassengerRappel_ProcessPostMovement(hook::FlexStruct* thisPtr)
{
	if (!thisPtr)
		return false;

	auto* ped = thisPtr->Get<hook::FlexStruct*>(0x10);
	if (!ped)
		return false;

	auto* vehicle = ped->Get<hook::FlexStruct*>(g_PedVehicleOffset);
	if (!vehicle)
		return false;

	const auto vehicleType = vehicle->Get<eVehicleType>(g_VehicleTypeOffset);
	if (vehicleType != VEHICLE_TYPE_HELI && vehicleType != VEHICLE_TYPE_BLIMP)
		return false;

	return g_CTaskHeliPassengerRappel_ProcessPostMovement(thisPtr);
}

static HookFunction hookFunction([]
{
	g_VehicleTypeOffset = *hook::get_pattern<int32_t>("41 83 BF ? ? ? ? 0B 74", 3);

	g_PedVehicleOffset = *hook::get_pattern<int32_t>("4C 39 BE ? ? ? ? 75 ? 41 8A C7", 3);

	g_CTaskHeliPassengerRappel_ProcessPostMovement = hook::trampoline(
		hook::get_pattern("48 8B C4 48 89 58 ? 55 56 57 41 54 41 55 41 56 41 57 48 8D A8 ? ? ? ? 48 81 EC ? ? ? ? 48 8B 79 ? 45 33 FF 0F 29 70 ? 48 8B 9F ? ? ? ? 0F 29 78 ? 48 8B F1 48 85 DB 0F 84"),
		CTaskHeliPassengerRappel_ProcessPostMovement);
});
