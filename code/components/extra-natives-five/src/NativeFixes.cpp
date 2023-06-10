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
#include <CrossBuildRuntime.h>

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

				if (windowIndex >= 0 && windowIndex <= 7)
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

static void FixGetVehiclePedIsIn()
{
	constexpr const uint64_t nativeHash = 0x9A9112A0FE9A4713; // GET_VEHICLE_PED_IS_IN

	auto handlerWrap = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handlerWrap)
	{
		return;
	}

	auto handler = *handlerWrap;

	auto location = hook::get_pattern<char>("80 8F ? ? ? ? 01 8B 86 ? ? ? ? C1 E8 1E");
	static uint32_t PedFlagsOffset = *reinterpret_cast<uint32_t*>(location + 9);
	static uint32_t LastVehicleOffset = *reinterpret_cast<uint32_t*>(location + 25);

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		auto lastVehicle = ctx.GetArgument<bool>(1);

		// If argument is true, call original handler as this behavior wasn't changed.
		if (lastVehicle)
		{
			handler(ctx);
			return;
		}

		auto pedHandle = ctx.GetArgument<uint32_t>(0);

		if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(pedHandle))
		{
			if (entity->IsOfType<CPed>())
			{
				if (auto lastVehicle = *reinterpret_cast<fwEntity**>((char*)entity + LastVehicleOffset))
				{
					auto pedFlags = *reinterpret_cast<uint32_t*>((char*)entity + PedFlagsOffset);

					if (pedFlags & (1 << 30))
					{
						ctx.SetResult(rage::fwScriptGuid::GetGuidFromBase(lastVehicle));
						return;
					}
				}
			}
		}

		ctx.SetResult(0);
	});
}

static int ReturnOne()
{
	return 1;
}

static void FixClearPedBloodDamage()
{
	// Find instruction block in Ped Resurrect function related to removing damage packs
	auto pedResurrectBlock = hook::get_pattern<char>("74 ? 48 8B ? D0 00 00 00 48 85 ? 74 ? 48 8B ? E8 ? ? ? ? 84 C0 74");
	static bool movOpcodeHasPrefix = *reinterpret_cast<uint8_t*>(pedResurrectBlock + 27) == 0x89;
	static uint32_t damagePackOffset = *reinterpret_cast<uint32_t*>(pedResurrectBlock + 28 + movOpcodeHasPrefix);

	// Navigate to subcall that checks if Ped is remote or if MP0_WALLET_BALANCE or BANK_BALANCE are >= 0
	auto isDmgPackRemovableFunc = *reinterpret_cast<uint32_t*>(pedResurrectBlock + 18) + pedResurrectBlock + 22;
	// Always satisfy positive balance check, this ensures visual & network state of player appearance won't get desynced
	hook::call(isDmgPackRemovableFunc + 13, ReturnOne);

	constexpr const uint64_t nativeHash = 0x8FE22675A5A45817; // CLEAR_PED_BLOOD_DAMAGE

	auto handlerWrap = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handlerWrap)
	{
		return;
	}
	auto handler = *handlerWrap;

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		// Run original handler first
		handler(ctx);

		// Zero out damage pack on network object level, the original handler doesn't touch this at all and causes desyncs
		auto pedHandle = ctx.GetArgument<uint32_t>(0);
		if (auto entity = rage::fwScriptGuid::GetBaseFromGuid(pedHandle))
		{
			if (entity->IsOfType<CPed>())
			{
				if (auto netObj = *reinterpret_cast<fwEntity**>((char*)entity + 0xD0))
				{
					auto damagePack = reinterpret_cast<uint32_t*>((char*)netObj + damagePackOffset);
					*damagePack = 0;
				}
			}
		}
	});
}

static void FixSetPedFaceFeature()
{
	constexpr const uint64_t nativeHash = 0x71A5C1DBA060049E; // _SET_PED_FACE_FEATURE

	auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!originalHandler)
	{
		return;
	}

	auto handler = *originalHandler;

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		// Check if the feature index is 18 (Chin Hole) or 19 (Neck Thickness)
		// These use 0.0 - 1.0 scales instead of the default -1.0 - 1.0
		auto index = ctx.GetArgument<uint32_t>(1);

		if (index == 18 || index == 19)
		{
			// Vanilla code ends up turning -1.0 into 1.0 in this case which feels counter-intuitive,
			// thus we manually bump negative values to 0.0
			auto scale = ctx.GetArgument<float>(2);
			if (scale < 0.0f)
			{
				ctx.SetArgument<float>(2, 0.0f);
			}
		}

		// Run the handler now that the scale factor is sanitized
		handler(ctx);
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

		// In b2699 R* changed this native, so now it ignores "lastVehicle" flag - fix it for compatibility.
		if (xbr::IsGameBuildOrGreater<2699>())
		{
			FixGetVehiclePedIsIn();
		}

		FixClearPedBloodDamage();

		FixSetPedFaceFeature();
	});
});
