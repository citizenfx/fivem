#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

enum FSM_Event
{
	OnEnter = 0,
	OnUpdate,
	OnExit,
};

enum FSM_Return
{
	FSM_Continue = 0,
	FSM_Quit,
};

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

static inline bool ValidateBoatTask(hook::FlexStruct* task)
{
	if (!task)
		return false;

	auto* entity = task->Get<hook::FlexStruct*>(0x10);
	if (!entity)
		return false;

	const auto type = entity->Get<eVehicleType>(g_VehicleTypeOffset);

	return type == VEHICLE_TYPE_BOAT || type == VEHICLE_TYPE_AMPHIBIOUS_AUTOMOBILE || type == VEHICLE_TYPE_AMPHIBIOUS_QUADBIKE;
}

static FSM_Return (*g_CTaskVehicleCruiseBoat_UpdateFSM)(hook::FlexStruct*, const int, const FSM_Event);

static FSM_Return CTaskVehicleCruiseBoat_UpdateFSM(hook::FlexStruct* thisPtr, const int32_t state, const FSM_Event event)
{
	if (!ValidateBoatTask(thisPtr))
		return FSM_Quit;

	return g_CTaskVehicleCruiseBoat_UpdateFSM(thisPtr, state, event);
}

static FSM_Return (*g_CTaskVehicleGoToBoat_UpdateFSM)(hook::FlexStruct*, const int, const FSM_Event);

static FSM_Return CTaskVehicleGoToBoat_UpdateFSM(hook::FlexStruct* thisPtr, const int32_t state, const FSM_Event event)
{
	if (!ValidateBoatTask(thisPtr))
		return FSM_Quit;

	return g_CTaskVehicleGoToBoat_UpdateFSM(thisPtr, state, event);
}

static FSM_Return (*g_CTaskVehicleGoToBoat_ProcessPreFSM)(hook::FlexStruct*);

static FSM_Return CTaskVehicleGoToBoat_ProcessPreFSM(hook::FlexStruct* thisPtr)
{
	if (!ValidateBoatTask(thisPtr))
		return FSM_Quit;

	return g_CTaskVehicleGoToBoat_ProcessPreFSM(thisPtr);
}

static HookFunction hookFunction([]
{
	// NOTE: also took from CrashFixes.ValidateSubmarineTasks.cpp
	g_VehicleTypeOffset = *hook::get_pattern<int32_t>("41 83 BF ? ? ? ? 0B 74", 3);

	g_CTaskVehicleCruiseBoat_UpdateFSM = hook::trampoline(hook::get_pattern("48 83 EC ? 85 D2 78 ? B8 ? ? ? ? 75 ? 44 3B C0 75 ? 8B D0 E8 ? ? ? ? 33 C0 48 83 C4 ? C3 3B D0 75 ? 45 85 C0 75 ? 48 8B 51 ? 48 83 C4"), CTaskVehicleCruiseBoat_UpdateFSM);
	
	g_CTaskVehicleGoToBoat_UpdateFSM = hook::trampoline(hook::get_pattern("48 83 EC ? 44 8B CA 48 8B 51 ? 45 85 C9 78 ? B8 ? ? ? ? 75"), CTaskVehicleGoToBoat_UpdateFSM);
	g_CTaskVehicleGoToBoat_ProcessPreFSM = hook::trampoline(hook::get_pattern("40 53 48 83 EC ? 44 8B 81 ? ? ? ? 48 8B 51 ? 48 8B D9 0F 29 74 24 ? 41 C1 E8 ? 48 8D 4C 24 ? 45 33 C9 41 80 E0 ? E8 ? ? ? ? 48 8D 8B ? ? ? ? 48 8D 54 24 ? ? ? ? E8 ? ? ? ? F3 0F 10 44 24"), CTaskVehicleGoToBoat_ProcessPreFSM);
});
