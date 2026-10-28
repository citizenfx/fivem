/*
 * This file is part of the Cfx project - https://cfx.re/
 *
 * See LICENSE in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"

#include <ScriptEngine.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <CoreConsole.h>
#include <nutsnbolts.h>

#include <algorithm>

static float g_vehicleVolume = 1.0f;

// "VEHICLES" - parent category of every vehicle sound
static constexpr uint32_t kVehiclesCategory = 0x74B31BE3;

namespace
{
	void** g_audCategoryControllerManager = nullptr;

	hook::thiscall_stub<char*(void*, uint32_t)> _createController([]()
	{
		return hook::get_call(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", 8));
	});

	char* CreateVehicleController()
	{
		auto mgr = g_audCategoryControllerManager ? *g_audCategoryControllerManager : nullptr;
		if (!mgr)
		{
			return nullptr;
		}

		return _createController(mgr, kVehiclesCategory);
	}
}

static char* GetVehicleController()
{
	static char* controller = nullptr;
	if (!controller)
	{
		controller = CreateVehicleController();
	}
	return controller;
}

static void ApplyVehicleVolume()
{
	if (auto controller = reinterpret_cast<hook::FlexStruct*>(GetVehicleController()))
	{
		controller->Set<float>(0, g_vehicleVolume);
		controller->Set<float>(4, 0.0f);
	}
}

static HookFunction hookFunction([]()
{
	g_audCategoryControllerManager = hook::get_address<void**>(hook::get_pattern("45 33 C0 BA 90 1C E2 44 E8", -4));

	OnMainGameFrame.Connect([]()
	{
		static bool applied = false;
		if (!applied && GetVehicleController())
		{
			ApplyVehicleVolume();
			applied = true;
		}
	});
});

static InitFunction initFunction([]()
{
	static ConVar<float> vehicleVolumeVar("profile_vehicleVolume", ConVar_Archive, 1.0f, &g_vehicleVolume);

	OnMainGameFrame.Connect([]()
	{
		static float last = -1.0f;
		if (g_vehicleVolume != last)
		{
			g_vehicleVolume = std::clamp(g_vehicleVolume, 0.0f, 1.0f);
			ApplyVehicleVolume();
			last = g_vehicleVolume;
		}
	});

	// no new natives will be added as of now but here they are in case that changes.

	// global vehicle audio volume: 0.0 silent .. 1.0 normal
	// kept disabled - gen8 is in feature freeze, no new natives for now
	// fx::ScriptEngine::RegisterNativeHandler("SET_AUDIO_VEHICLE_VOLUME", [](fx::ScriptContext& context)
	// {
	// 	g_vehicleVolume = std::clamp(context.GetArgument<float>(0), 0.0f, 1.0f);
	// 	ApplyVehicleVolume();
	// });

	// fx::ScriptEngine::RegisterNativeHandler("GET_AUDIO_VEHICLE_VOLUME", [](fx::ScriptContext& context)
	// {
	// 	context.SetResult<float>(g_vehicleVolume);
	// });
});
