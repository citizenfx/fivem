/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <Local.h>

#include <array>

#include <ScriptEngine.h>
#include <Hooking.h>
#include <scrEngine.h>
#include <CrossBuildRuntime.h>
#include "RageParser.h"
#include "Resource.h"
#include "ScriptWarnings.h"

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

struct FireInfoEntry
{
	fwEntity*& entity()
	{
		return *reinterpret_cast<fwEntity**>(reinterpret_cast<uintptr_t>(this) + 0x30);
	}

	uint8_t& flags()
	{
		return *reinterpret_cast<uint8_t*>(reinterpret_cast<uintptr_t>(this) + 0x74);
	}

private:
	std::array<uint8_t, 0xF0> m_pad{};
};

std::array<FireInfoEntry, 128>* g_fireInstances;

void FreeOrphanFireEntries()
{
	for (auto& i : *g_fireInstances)
	{
		if (!i.entity() && i.flags() == 0x4)
		{
			i.flags() &= ~0x4;
		}
	}
}

static void FixStartEntityFire()
{
	constexpr const uint64_t nativeHash = 0xF6A9D9708F6F23DF; // START_ENTITY_FIRE

	auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!originalHandler)
	{
		return;
	}

	auto handler = *originalHandler;

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		FreeOrphanFireEntries();
		handler(ctx);
	});
}

static void FixStopEntityFire()
{
	constexpr const uint64_t nativeHash = 0x7F0DD2EBBB651AFF; // STOP_ENTITY_FIRE

	const auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!originalHandler)
	{
		return;
	}

	const auto handler = *originalHandler;

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		FreeOrphanFireEntries();

		const auto handle = ctx.GetArgument<uint32_t>(0);
		const auto entity = rage::fwScriptGuid::GetBaseFromGuid(handle);

		if (!entity)
		{
			return handler(ctx);
		}

		auto entries = std::vector<FireInfoEntry*>{};

		for (auto& i : *g_fireInstances)
		{
			if (i.entity() == entity)
			{
				entries.push_back(&i);
			}
		}

		handler(ctx);

		for (const auto i : entries)
		{
			i->entity() = 0x0;
			i->flags() &= ~0x4;
		}
	});
}

static void FixPedCombatAttributes()
{
	const auto structDef = rage::GetStructureDefinition("CCombatInfo");

	if (!structDef)
	{
		trace("Couldn't find struct definition for CCombatInfo!\n");
		return;
	}

	static uint32_t attributesCount = 0;

	for (const auto member : structDef->m_members)
	{
		if (member->m_definition && member->m_definition->hash == HashRageString("BehaviourFlags"))
		{
			attributesCount = member->m_definition->enumElemCount;
			break;
		}
	}

	if (!attributesCount)
	{
		trace("Couldn't get max enum size for BehaviourFlags!\n");
		return;
	}

	constexpr const uint64_t nativeHash = 0x9F7794730795E019; // SET_PED_COMBAT_ATTRIBUTES

	auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!originalHandler)
	{
		return;
	}

	auto handler = *originalHandler;

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		auto attributeIndex = ctx.GetArgument<uint32_t>(1);

		if (attributeIndex >= attributesCount)
		{
			fx::scripting::Warningf("natives", "SET_PED_COMBAT_ATTRIBUTES: invalid attribute index was passed (%d), should be from 0 to %d\n", attributeIndex, attributesCount - 1);
			return;
		}

		handler(ctx);
	});
}

static int32_t g_maxHudColours;
static void FixReplaceHudColour()
{
	constexpr uint64_t REPLACE_HUD_COLOUR = 0x1CCC708F0F850613;
	constexpr uint64_t REPLACE_HUD_COLOUR_WITH_RGBA = 0xF314CF4F0211894E;

	for (uint64_t nativeHash : { REPLACE_HUD_COLOUR, REPLACE_HUD_COLOUR_WITH_RGBA })
	{
		auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!originalHandler)
		{
			continue;
		}

		auto handler = *originalHandler;
		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			auto hudColorIndex = ctx.GetArgument<int32_t>(0);
			if (hudColorIndex < 0 || hudColorIndex > g_maxHudColours)
			{
				fx::scripting::Warningf("natives", "Invalid HUD_COLOUR index was passed (%d), should be from 0 to %d\n", hudColorIndex, g_maxHudColours);
				return;
			}

			handler(ctx);
		});
	}
}

static int32_t g_numMarkerTypes;
static void FixDrawMarker()
{
	constexpr uint64_t DRAW_MARKER = 0x28477EC23D892089;
	constexpr uint64_t DRAW_MARKER_EX = 0xE82728F0DE75D13A;

	for (uint64_t nativeHash : { DRAW_MARKER, DRAW_MARKER_EX })
	{
		auto originalHandler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!originalHandler)
		{
			continue;
		}

		auto handler = *originalHandler;
		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			auto markerType = ctx.GetArgument<int32_t>(0);
			if (markerType < 0 || markerType >= g_numMarkerTypes)
			{
				fx::scripting::Warningf("natives", "Invalid MARKER_TYPE index was passed (%d), should be from 0 to %d\n", markerType, g_numMarkerTypes - 1);
				return;
			}

			handler(ctx);
		});
	}
}

static HookFunction hookFunction([]()
{
	g_fireInstances = (std::array<FireInfoEntry, 128>*)(hook::get_address<uintptr_t>(hook::get_pattern("74 47 48 8D 0D ? ? ? ? 48 8B D3", 2), 3, 7) + 0x10);
	g_maxHudColours = *hook::get_pattern<int32_t>("81 F9 ? ? ? ? 77 5A 48 89 5C 24", 2);
	g_numMarkerTypes = *hook::get_pattern<int32_t>("BE FF FF FF DF 41 BF 00 00 FF 0F 41 BC FF FF FF BF", -4);

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

		FixStartEntityFire();

		FixStopEntityFire();

		FixPedCombatAttributes();

		FixReplaceHudColour();

		FixDrawMarker();
	});
});
