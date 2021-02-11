/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <Local.h>

#include <ScriptEngine.h>
#include <Hooking.h>
#include <scrEngine.h>

static hook::cdecl_stub<bool(fwEntity*, uint8_t)> isVehicleWindowValid([]()
{
	return hook::get_pattern("85 D2 78 ? 48 8B 49 ? 4C 8B 81", -2);
});

static void FixVehicleWindowNatives()
{
	// native hash, should set result (not a void)
	std::pair<uint64_t, bool> nativesToPatch[] = {
		{ 0x46E571A0E20D01F1, true },  // IS_VEHICLE_WINDOW_INTACT
		{ 0x772282EBEB95E682, false }, // FIX_VEHICLE_WINDOW
		{ 0xA711568EEDB43069, false }, // REMOVE_VEHICLE_WINDOW
		{ 0x7AD9E6CE657D69E3, false }, // ROLL_DOWN_WINDOW
		{ 0x602E548F46E24D59, false }, // ROLL_UP_WINDOW
		{ 0x9E5B5E4D2CCD2259, false }, // SMASH_VEHICLE_WINDOW
	};

	for (auto native : nativesToPatch)
	{
		auto handler = fx::ScriptEngine::GetNativeHandler(native.first);

		if (!handler)
		{
			trace("Couldn't find 0x%08x handler to hook!\n", native.first);
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(native.first, [=](fx::ScriptContext& ctx)
		{
			auto vehicleHandle = ctx.GetArgument<uint32_t>(0);
			auto vehicle = rage::fwScriptGuid::GetBaseFromGuid(vehicleHandle);

			if (vehicle && vehicle->IsOfType<CVehicle>())
			{
				auto windowIndex = ctx.GetArgument<int>(1);

				if (isVehicleWindowValid(vehicle, windowIndex))
				{
					return (*handler)(ctx);
				}
			}

			if (native.second)
			{
				ctx.SetResult<bool>(false);
			}
		});
	}
}

static void FixClockTimeOverrideNative()
{
	uint64_t nativeHash = 0xE679E3E06E363892; // NETWORK_OVERRIDE_CLOCK_TIME

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		trace("Couldn't find 0x%08x handler to hook!\n", nativeHash);
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [=](fx::ScriptContext& ctx)
	{
		auto hours = ctx.GetArgument<uint32_t>(0);
		auto minutes = ctx.GetArgument<uint32_t>(1);
		auto seconds = ctx.GetArgument<uint32_t>(2);

		if (hours < 24 && minutes < 60 && seconds < 60)
		{
			(*handler)(ctx);
		}
	});
}

static HookFunction hookFunction([]()
{
	rage::scrEngine::OnScriptInit.Connect([]()
	{
		// Most of vehicle window related natives have no checks for passed window index is valid
		// for specified vehicle, passing wrong values lead to native execution exception.
		FixVehicleWindowNatives();

		// Passing wrong clock time values to override native leads to sudden crash.
		FixClockTimeOverrideNative();
	});
});
