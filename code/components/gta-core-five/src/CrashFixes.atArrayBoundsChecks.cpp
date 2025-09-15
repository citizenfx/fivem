#include "StdInc.h"

#include "atArray.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

namespace
{

enum DispatchType
{
	DT_Invalid = 0,
	DT_PoliceAutomobile,
	DT_PoliceHelicopter,
	DT_FireDepartment,
	DT_SwatAutomobile,
	DT_AmbulanceDepartment,
	DT_PoliceRiders,
	DT_PoliceVehicleRequest,
	DT_PoliceRoadBlock,
	DT_PoliceAutomobileWaitPulledOver,
	DT_PoliceAutomobileWaitCruising,
	DT_Gangs,
	DT_SwatHelicopter,
	DT_PoliceBoat,
	DT_ArmyVehicle,
	DT_BikerBackup,
	DT_Assassins,
	DT_Max
};

// ---------------------------------------------------------------------------
// Fix: Validate array index in SetNumPedsToSpawn
// "m_aNumPedsToSpawn" is an atRangeArray of size DT_Max.
// Ensure we are not writing out of bounds by discarding invalid dispatch types.
// ---------------------------------------------------------------------------

void (*Orig_CWantedResponseOverrides_SetNumPedsToSpawn)(void*, DispatchType, int8_t);

void CWantedResponseOverrides_SetNumPedsToSpawn(void* self, DispatchType nDispatchType, int8_t iNumPedsToSpawn)
{
	if (nDispatchType < DT_Invalid || nDispatchType >= DT_Max)
	{
		trace("CWantedResponseOverrides::SetNumPedsToSpawn: invalid dispatch type %d\n", nDispatchType);
		return;
	}

	Orig_CWantedResponseOverrides_SetNumPedsToSpawn(self, nDispatchType, iNumPedsToSpawn);
}

// ---------------------------------------------------------------------------
// Fix: Validate array index in SetEnabled
// "sm_popZoneSearchInfos" is an atArray whose capacity is determined by the zone data loaded from mounted IPL files.
// Ensure we are not writing out of bounds by discarding invalid zone IDs.
// ---------------------------------------------------------------------------

atArray<void*>* CPopZones_sm_popZoneSearchInfos;

void (*Orig_CPopZones_SetEnabled)(int, bool);

void CPopZones_SetEnabled(int zone, bool bEnabled)
{
	if (zone < 0 || zone >= CPopZones_sm_popZoneSearchInfos->GetCount())
	{
		trace("CPopZones::SetEnabled: invalid zone index %d\n", zone);
		return;
	}

	Orig_CPopZones_SetEnabled(zone, bEnabled);
}

}

static HookFunction hookFunction([]
{
	Orig_CWantedResponseOverrides_SetNumPedsToSpawn = hook::trampoline(hook::get_pattern("48 63 C2 44 88 04 08"), CWantedResponseOverrides_SetNumPedsToSpawn);

	CPopZones_sm_popZoneSearchInfos = hook::get_address<atArray<void*>*>(hook::get_pattern("48 8D 0D ? ? ? ? F3 0F 2C C6"), 3, 7);
	Orig_CPopZones_SetEnabled = hook::trampoline(hook::get_pattern("8B C1 80 E2 01 48 8D"), CPopZones_SetEnabled);
});
