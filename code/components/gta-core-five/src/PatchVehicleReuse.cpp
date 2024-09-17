#include <StdInc.h>
#include <Hooking.h>
#include <CrossBuildRuntime.h>

//
// GH-2425: Ensure the CVehicle flag associated with "Using Pretend Occupants" 
// is reset when a vehicle is being recycled from the reuse pool. At the moment 
// FXServer does the repopulate the entire synctree with calls to CREATE_VEHICLE_SERVER_SETTER
// causing this value to leak from previous vehicles.
//

class /*rage::*/ Matrix34;
class CVehicle
{
public:
	inline static ptrdiff_t kFlagsOffset;
	inline static uint8_t kPretendOccupantsMask;

public:
	inline void ClearUsesPretendOccupants()
	{
		auto location = reinterpret_cast<uint8_t*>(this) + kFlagsOffset;
		*location &= ~kPretendOccupantsMask;
	}
};

static bool (*g_origRecycleVehicle)(CVehicle*, Matrix34*, int32_t, int32_t, bool, bool);
static bool CVehiclePopulation_RecycleVehicle(CVehicle* vehicle, Matrix34* matrix, int32_t poptype, int32_t ownedBy, bool p5, bool p6)
{
	if (g_origRecycleVehicle(vehicle, matrix, poptype, ownedBy, p5, p6))
	{
		vehicle->ClearUsesPretendOccupants();
		return true;
	}

	return false;
}

static HookFunction hookFunction([]()
{
	{
		auto location = hook::get_pattern<char>("45 8B CE C6 44 24 ? ? 48 8B F8 C6 44 24", 0x10);
		hook::set_call(&g_origRecycleVehicle, location);
		hook::call(location, CVehiclePopulation_RecycleVehicle);
	}

	{
		auto location = hook::get_pattern<char>("48 83 EC 30 F6 81 ? ? ? ? ? 48 8B D9 74 04", 0x4);
		CVehicle::kFlagsOffset = *reinterpret_cast<int32_t*>(location + 2);
		CVehicle::kPretendOccupantsMask = *reinterpret_cast<uint8_t*>(location + 6);
	}
});
