#include "StdInc.h"

#include "Hooking.h"
#include "Hooking.Patterns.h"
#include "Hooking.Stubs.h"

static constexpr int32_t kMaxVehiclePlayerLockSlots = 64;

static int32_t* g_vehiclePlayerLockCount;

static uint32_t (*g_AllocateVehiclePlayerLockSlot)();

static uint32_t AllocateVehiclePlayerLockSlot()
{
	if (*g_vehiclePlayerLockCount >= kMaxVehiclePlayerLockSlots)
	{
		return 0xFFFFFFFF;
	}

	return g_AllocateVehiclePlayerLockSlot();
}

static HookFunction hookFunction([]
{
	auto location = hook::get_pattern<uint8_t>("48 83 EC ? 4C 63 0D ? ? ? ? 33 C9 33 D2 41 BB FF 00 00 00");

	g_vehiclePlayerLockCount = hook::get_address<int32_t*>(location + 4, 3, 7);

	g_AllocateVehiclePlayerLockSlot = hook::trampoline(location, AllocateVehiclePlayerLockSlot);
});
