#include "StdInc.h"

#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

class CTaskInVehicleSeatShuffle
{
public:
	char m_Padding[0x36];
	int8_t m_TaskState;

	char m_Padding2[0xE9];
	void* m_Vehicle;
	void* m_JackedPed;
	int32_t m_TargetSeatIndex;
	int32_t m_CurrentSeatIndex;
};
static_assert(sizeof(CTaskInVehicleSeatShuffle) == 0x138, "CTaskInVehicleSeatShuffle has wrong size!");

static hook::cdecl_stub<void*(void*, int32_t)> g_CVehicle_GetSeatAnimationInfo([]
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 85 C0 74 ? 8B 78 ? C1 EF"));
});

enum FSM_Return
{
	FSM_Continue = 0,
	FSM_Quit,
};

static FSM_Return (*g_CTaskInVehicleSeatShuffle_ProcessPreFSM)(CTaskInVehicleSeatShuffle*);

static FSM_Return CTaskInVehicleSeatShuffle_ProcessPreFSM(CTaskInVehicleSeatShuffle* thisPtr)
{
	// NOTE: we are calling original first to make sure the vehicle is still valid
	FSM_Return result = g_CTaskInVehicleSeatShuffle_ProcessPreFSM(thisPtr);

	if (result == FSM_Quit)
		return FSM_Quit;

    if (thisPtr->m_TaskState <= 4 && thisPtr->m_TargetSeatIndex == -1)
		return result;

	const bool invalidTargetSeat = !g_CVehicle_GetSeatAnimationInfo(
		thisPtr->m_Vehicle,
		thisPtr->m_TargetSeatIndex);

	if (invalidTargetSeat)
		return FSM_Quit;

	return result;
}

static HookFunction hookFunction([]
{
	g_CTaskInVehicleSeatShuffle_ProcessPreFSM = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 57 48 83 EC ? 48 8B 59 ? 48 8B 91"), CTaskInVehicleSeatShuffle_ProcessPreFSM);
});
