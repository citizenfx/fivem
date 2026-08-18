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

using TaskUpdateFSM = FSM_Return (*)(hook::FlexStruct*, const int32_t, const FSM_Event);

static int32_t g_VehicleTypeOffset;

template<eVehicleType... AllowedTypes>
static bool ValidateVehicleTask(hook::FlexStruct* task)
{
	if (!task)
		return false;

	auto* entity = task->Get<hook::FlexStruct*>(0x10);
	if (!entity)
		return false;

	const auto type = entity->Get<eVehicleType>(g_VehicleTypeOffset);
	return ((type == AllowedTypes) || ...);
}

template<TaskUpdateFSM& Original, bool (*Validate)(hook::FlexStruct*)>
static FSM_Return ValidatedUpdateFSM(hook::FlexStruct* thisPtr, const int32_t state, const FSM_Event event)
{
	if (!Validate(thisPtr))
		return FSM_Quit;

	return Original(thisPtr, state, event);
}

template<TaskUpdateFSM& Original, bool (*Validate)(hook::FlexStruct*)>
static void HookVehicleTask(const char* pattern)
{
	Original = hook::trampoline(hook::get_pattern(pattern), ValidatedUpdateFSM<Original, Validate>);
}

static TaskUpdateFSM g_CTaskVehicleGoToPlane_UpdateFSM;
static TaskUpdateFSM g_CTaskVehicleLandPlane_UpdateFSM;
static TaskUpdateFSM g_CTaskVehiclePlaneChase_UpdateFSM;

static TaskUpdateFSM g_CTaskVehicleGoToHelicopter_UpdateFSM;
static TaskUpdateFSM g_CTaskVehiclePoliceBehaviourHelicopter_UpdateFSM;
static TaskUpdateFSM g_CTaskVehicleHeliProtect_UpdateFSM;

static constexpr auto kValidatePlaneTask = ValidateVehicleTask<VEHICLE_TYPE_PLANE>;
static constexpr auto kValidateHeliTask = ValidateVehicleTask<VEHICLE_TYPE_HELI, VEHICLE_TYPE_BLIMP>;

static HookFunction hookFunction([]
{
	// NOTE: also took from CrashFixes.ValidateSubmarineTasks.cpp
	g_VehicleTypeOffset = *hook::get_pattern<int32_t>("41 83 BF ? ? ? ? 0B 74", 3);

	HookVehicleTask<g_CTaskVehicleGoToPlane_UpdateFSM, kValidatePlaneTask>(
		"48 83 EC ? 8B C2 48 8B 51 ? 85 C0 78 ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? 33 C0 48 83 C4 ? C3 41 83 F8 ? 75 ? 48 83 C4 ? E9 ? ? ? ? B8 ? ? ? ? EB ? ? 48 83 EC ? 8B C2 48 8B 51");

	HookVehicleTask<g_CTaskVehicleLandPlane_UpdateFSM, kValidatePlaneTask>(
		"48 83 EC ? 85 D2 78 ? B8 ? ? ? ? 75 ? 44 3B C0 75 ? 8B D0 E8 ? ? ? ? 33 C0 48 83 C4 ? C3 3B D0 75 ? 45 85 C0 75 ? 48 83 C4");

	HookVehicleTask<g_CTaskVehiclePlaneChase_UpdateFSM, kValidatePlaneTask>(
		"48 83 EC ? 85 D2 78 ? B8 ? ? ? ? 75 ? 44 3B C0 75 ? 8B D0 E8 ? ? ? ? EB ? 3B D0 75 ? 45 85 C0 75 ? E8 ? ? ? ? EB ? 44 3B C0 75 ? 48 83 C4 ? E9 ? ? ? ? 83 FA ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? EB ? 44 3B C0 75 ? 48 83 C4 ? E9 ? ? ? ? 41 83 F8");

	HookVehicleTask<g_CTaskVehicleGoToHelicopter_UpdateFSM, kValidateHeliTask>(
		"48 83 EC ? 8B C2 48 8B 51 ? 85 C0 78 ? 75 ? 45 85 C0 75 ? E8 ? ? ? ? 33 C0 48 83 C4 ? C3 41 83 F8 ? 75 ? 48 83 C4 ? E9 ? ? ? ? 41 83 F8");

	HookVehicleTask<g_CTaskVehiclePoliceBehaviourHelicopter_UpdateFSM, kValidateHeliTask>(
		"48 83 EC ? 8B C2 48 8B 51 ? 85 C0 78 ? 75 ? 41 83 F8 ? 75 ? E8");

	HookVehicleTask<g_CTaskVehicleHeliProtect_UpdateFSM, kValidateHeliTask>(
		"48 83 EC ? 85 D2 78 ? B8 ? ? ? ? 75 ? 44 3B C0 75 ? 8B D0 E8 ? ? ? ? 33 C0 48 83 C4 ? C3 3B D0 75 ? 45 85 C0 75 ? 48 8B 51 ? E8 ? ? ? ? EB ? 44 3B C0 75 ? 48 8B 51 ? 48 83 C4 ? E9 ? ? ? ? 48 83 EC");
});
