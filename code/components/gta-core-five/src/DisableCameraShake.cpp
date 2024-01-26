/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include <StdInc.h>
#include <Hooking.h>
#include <CoreConsole.h>

#include "Hooking.Stubs.h"

static bool g_disableCameraShake;

static void (*g_origUpdateCameraVehicleHighSpeedShake)(void*);
static void UpdateCameraVehicleHighSpeedShake(void* a1)
{
	if (!g_disableCameraShake)
	{
		g_origUpdateCameraVehicleHighSpeedShake(a1);
	}
}

static void (*g_origUpdateCameraPedRunningShake)(void*, void*, void*);
static void UpdateCameraPedRunningShake(void* a1, void* a2, void* a3)
{
	if (!g_disableCameraShake)
	{
		g_origUpdateCameraPedRunningShake(a1, a2, a3);
	}
}

static InitFunction initFunction([]()
{
	static ConVar<bool> disableCameraShakeVar("cam_disableCameraShake", ConVar_Archive, false, &g_disableCameraShake);
});

static HookFunction hookFunction([]
{
	{
		auto location = hook::get_pattern("48 8B D9 48 81 C7 ? ? ? ? 8B ? ? 85", -31);
		g_origUpdateCameraVehicleHighSpeedShake = hook::trampoline(location, UpdateCameraVehicleHighSpeedShake);
	}

	{
		auto location = hook::get_pattern("57 48 81 EC ? ? ? ? 48 8B B9 ? ? ? ? 0F 29 70 E8 0F 29 78 D8 48 81 C7", -11);
		g_origUpdateCameraPedRunningShake = hook::trampoline(location, UpdateCameraPedRunningShake);
	}
});
