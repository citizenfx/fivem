#include "StdInc.h"

#include "CrossBuildRuntime.h"
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

// PatchVehicleHoodCamera.cpp
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

static int g_vehicleTypeOffset;

static std::once_flag traceOnceFlag;

static FSM_Return (*orig_CTaskVehicleGoToSubmarine_UpdateFSM)(hook::FlexStruct*, const int, const FSM_Event);

static FSM_Return CTaskVehicleGoToSubmarine_UpdateFSM(hook::FlexStruct* self, const int iState, const FSM_Event iEvent)
{
	auto m_pEntity = self->Get<hook::FlexStruct*>(0x10);

	if (!m_pEntity)
	{
		return FSM_Quit;
	}

	const auto vehicleType = m_pEntity->Get<eVehicleType>(g_vehicleTypeOffset);
	if (vehicleType != VEHICLE_TYPE_SUBMARINE && vehicleType != VEHICLE_TYPE_SUBMARINECAR)
	{
		std::call_once(traceOnceFlag, []
		{
			trace("Mitigated submarine task on invalid vehicle type [CFX-3297]\n");
		});

		return FSM_Quit;
	}

	return orig_CTaskVehicleGoToSubmarine_UpdateFSM(self, iState, iEvent);
}

static FSM_Return (*orig_CTaskVehiclePlayerDriveSubmarineCar_UpdateFSM)(hook::FlexStruct*, const int, const FSM_Event);

static FSM_Return CTaskVehiclePlayerDriveSubmarineCar_UpdateFSM(hook::FlexStruct* self, const int iState, const FSM_Event iEvent)
{
	auto m_pEntity = self->Get<hook::FlexStruct*>(0x10);

	if (!m_pEntity)
	{
		return FSM_Quit;
	}

	const auto vehicleType = m_pEntity->Get<eVehicleType>(g_vehicleTypeOffset);
	if (vehicleType != VEHICLE_TYPE_SUBMARINE && vehicleType != VEHICLE_TYPE_SUBMARINECAR)
	{
		std::call_once(traceOnceFlag, []
		{
			trace("Mitigated submarine task on invalid vehicle type [CFX-3297]\n");
		});

		return FSM_Quit;
	}

	return orig_CTaskVehiclePlayerDriveSubmarineCar_UpdateFSM(self, iState, iEvent);
}

static HookFunction hookFunction([]
{
	if (!xbr::IsGameBuildOrGreater<2189>())
	{
		return;
	}

	// Stolen from "VehicleExtraNatives.cpp"
	g_vehicleTypeOffset = *hook::get_pattern<int>("41 83 BF ? ? ? ? 0B 74", 3);

	orig_CTaskVehicleGoToSubmarine_UpdateFSM = hook::trampoline(hook::get_pattern("48 83 EC ? 8B C2 48 8B 51 ? 85 C0 78 ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? 33 C0 48 83 C4 ? C3 41 83 F8 ? 75 ? 48 83 C4 ? E9 ? ? ? ? 83 F8 ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? EB ? 41 83 F8 ? 75 ? 48 83 C4 ? E9 ? ? ? ? 83 F8 ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? EB ? 41 83 F8 ? 75 ? 48 83 C4 ? E9 ? ? ? ? 83 F8"), CTaskVehicleGoToSubmarine_UpdateFSM);

	orig_CTaskVehiclePlayerDriveSubmarineCar_UpdateFSM = hook::trampoline(hook::get_pattern("48 83 EC ? 8B C2 48 8B 51 ? 45 33 D2 80 8A"), CTaskVehiclePlayerDriveSubmarineCar_UpdateFSM);
});
