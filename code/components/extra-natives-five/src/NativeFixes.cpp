/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <Local.h>

#include <array>
#include <vector>

#include <ScriptEngine.h>
#include <Hooking.h>
#include <scrEngine.h>
#include <CrossBuildRuntime.h>
#include "RageParser.h"
#include "Resource.h"
#include "ScriptWarnings.h"
#include <Train.h>
#include <CrashFixes.FakeParachuteProp.h>
#include "ropeManager.h"

static void BlockForbiddenNatives()
{
	std::vector<uint64_t> nativesToBlock = rage::scrEngine::GetBlockedNatives();
	for (auto native: nativesToBlock)
	{
		auto origHandler = fx::ScriptEngine::GetNativeHandler(native);
		if (!origHandler)
		{
			continue;
		}

		fx::ScriptEngine::RegisterNativeHandler(native, [=](fx::ScriptContext& ctx)
		{
			if (rage::scrEngine::GetStoryMode())
			{
				origHandler(ctx);
			}
			else
			{
				ctx.SetResult<uintptr_t>(0);
			}
		});
	}
}

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
					return handler(ctx);
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
			handler(ctx);
		}
	});
}

static void FixGetVehiclePedIsIn()
{
	constexpr const uint64_t nativeHash = 0x9A9112A0FE9A4713; // GET_VEHICLE_PED_IS_IN

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

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

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

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

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

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

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		FreeOrphanFireEntries();
		handler(ctx);
	});
}

static void FixStopEntityFire()
{
	constexpr const uint64_t nativeHash = 0x7F0DD2EBBB651AFF; // STOP_ENTITY_FIRE

	const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

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

	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

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
		auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			continue;
		}

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
		auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			continue;
		}

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

static void FixApplyForceToEntity()
{
	// ApplyForceToEntity checks if the entity is valid, but then does some stuff outside of that check

	constexpr const uint64_t nativeHash = 0xC5F68BE9613E2D18; // APPLY_FORCE_TO_ENTITY

	const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		const auto handle = ctx.GetArgument<uint32_t>(0);
		const auto entity = rage::fwScriptGuid::GetBaseFromGuid(handle);

		if (entity && entity->IsOfType<CPhysical>())
		{
			handler(ctx);
		}
	});
}

static void FixIsBitSet()
{
	constexpr const uint64_t nativeHash = 0xE2D0C323A1AE5D85; // IS_BIT_SET

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [](fx::ScriptContext& ctx)
	{
		bool result = false;

		auto value = ctx.GetArgument<uint32_t>(0);
		auto offset = ctx.GetArgument<int>(1);

		if (offset < 32)
		{
			result = (value & (1 << offset)) != 0;
		}

		ctx.SetResult<int>(result);
	});
}

static hook::cdecl_stub<bool(uint32_t* mi)> hasModelLoaded([]()
{
	return hook::get_call(hook::get_pattern("25 FF FF FF 3F 89 45 6F E8 ? ? ? ? 84 C0", 8));
});

static rage::CTrainConfigData* g_trainConfigData;

static void FixMissionTrain()
{
	constexpr uint64_t CREATE_MISSION_TRAIN = 0x63C6CCA8E68AE8C8;

	auto handler = fx::ScriptEngine::GetNativeHandler(CREATE_MISSION_TRAIN);
	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(CREATE_MISSION_TRAIN, [handler](fx::ScriptContext& ctx)
	{
		auto variation = ctx.GetArgument<int>(0);

		if (variation < 0 || variation >= g_trainConfigData->m_trainConfigs.GetCount())
		{
			fx::scripting::Warningf("natives", "Invalid train variation index was passed to CREATE_MISSION_TRAIN (%i), should be from 0 to %i\n", variation, g_trainConfigData->m_trainConfigs.GetCount() - 1);
			ctx.SetResult<int>(0);
			return;
		}

		rage::CTrainConfig config = g_trainConfigData->m_trainConfigs.Get(variation);

		// Prevent the native from executing if one any of the required models are not in memory
		for (auto& carriage : config.m_carriages)
		{
			rage::fwModelId idx{ 0 };
			rage::fwArchetypeManager::GetArchetypeFromHashKey(carriage.m_hash, idx);
			if (!hasModelLoaded(&idx.value))
			{
				fx::scripting::Warningf("natives", "Failed to spawn mission train as carriage hash '%i' is not loaded\n", carriage.m_hash);
				ctx.SetResult<int>(0);
				return;
			}	
		}

		// Prevent the native from executing if there are no tracks available. This won't crash the game but can give a confusing error.
		if (rage::CTrainTrack::AreAllTracksDisabled())
		{
			fx::scripting::Warningf("natives", "Failed to spawn mission train as there are no tracks enabled\n");
			ctx.SetResult<int>(0);
			return;
		}

		handler(ctx);
	});
}

static void FixAddRopeNative()
{
	constexpr const uint64_t ADD_ROPE = 0xE832D760399EB220;
	auto handler = fx::ScriptEngine::GetNativeHandler(ADD_ROPE);

	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(ADD_ROPE, [handler](fx::ScriptContext& ctx)
	{
		rage::ropeDataManager* manager = rage::ropeDataManager::GetInstance();
		if (manager)
		{
			auto ropeIndex = ctx.GetArgument<int>(7);
			if (ropeIndex >= 0 && ropeIndex < manager->typeData.GetCount())
			{
				return handler(ctx);
			}
			else
			{
				fx::scripting::Warningf("natives", "Invalid rope type was passed to ADD_ROPE (%d), should be from 0 to %d\n", ropeIndex, manager->typeData.GetCount() - 1);
			}
		}
		ctx.SetResult(0);
	});
}

// PatchVehicleHoodCamera.cpp
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

static int g_vehicleTypeOffset;

using CHeli_BreakOffRotor_t = void(__fastcall*)(fwEntity* heli);
static CHeli_BreakOffRotor_t CHeli_BreakOffMainRotor;
static CHeli_BreakOffRotor_t CHeli_BreakOffTailRotor;

static void SetHeliRotorHealthHandler(const uint64_t nativeHash, void(*breakOffRotorFunc)(rage::fwEntity*))
{
	auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);

	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler, breakOffRotorFunc](fx::ScriptContext& ctx)
	{
		handler(ctx);

		const auto rotorHealth = ctx.GetArgument<float>(1);
		if (rotorHealth > 0.0f && !std::isnan(rotorHealth))
		{
			return;
		}

		const auto vehicleHandle = ctx.GetArgument<uint32_t>(0);
		const auto vehicle = rage::fwScriptGuid::GetBaseFromGuid(vehicleHandle);
		if (vehicle && vehicle->IsOfType<CVehicle>())
		{
			const auto vehicleType = *reinterpret_cast<eVehicleType*>((char*)vehicle + g_vehicleTypeOffset);
			if (vehicleType == VEHICLE_TYPE_HELI || vehicleType == VEHICLE_TYPE_BLIMP)
			{
				breakOffRotorFunc(vehicle);
			}
		}
	});
}

static void FixSetHeliRotorHealth()
{
	constexpr uint64_t nativeHash = 0x4056EA1105F5ABD7; // _SET_HELI_MAIN_ROTOR_HEALTH
	constexpr uint64_t nativeHash2 = 0xFE205F38AAA58E5B; // _SET_HELI_TAIL_ROTOR_HEALTH

	SetHeliRotorHealthHandler(nativeHash, CHeli_BreakOffMainRotor);
	SetHeliRotorHealthHandler(nativeHash2, CHeli_BreakOffTailRotor);
}

static void FixSetPlayerParachuteModelOverride()
{
	// Make SET_PLAYER_PARACHUTE_MODEL_OVERRIDE() use a whitelist of model hashes.
	// This prevents a vulnerability that could cause a client to crash.

	constexpr const uint64_t nativeHash = 0x977DB4641F6FC3DB; // SET_PLAYER_PARACHUTE_MODEL_OVERRIDE

	const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		uint32_t modelHash = ctx.GetArgument<uint32_t>(1);
		if (IsParachuteModelAuthorized(modelHash))
		{
			handler(ctx);
		}
	});
}

static void FixSetPlayerParachutePackModelOverride()
{
	// Make SET_PLAYER_PARACHUTE_PACK_MODEL_OVERRIDE() use a whitelist of model hashes.
	// This prevents a vulnerability that could cause a client to crash.

	constexpr const uint64_t nativeHash = 0xDC80A4C2F18A2B64; // SET_PLAYER_PARACHUTE_PACK_MODEL_OVERRIDE

	const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
	if (!handler)
	{
		return;
	}

	fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
	{
		uint32_t modelHash = ctx.GetArgument<uint32_t>(1);
		if (IsParachutePackModelAuthorized(modelHash))
		{
			handler(ctx);
		}
	});
}

static void FixActionscriptFlagNatives()
{
	{
		constexpr const uint64_t nativeHash = 0xE3B05614DCE1D014; // GET_GLOBAL_ACTIONSCRIPT_FLAG

		const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			int32_t flagIndex = ctx.GetArgument<uint32_t>(0);
			if (flagIndex < 0)
			{
				ctx.SetResult<int>(0);
				return;
			}

			handler(ctx);
		});
	}

	{
		constexpr const uint64_t nativeHash = 0xB99C4E4D9499DF29; // RESET_GLOBAL_ACTIONSCRIPT_FLAG

		const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			int32_t flagIndex = ctx.GetArgument<uint32_t>(0);
			if (flagIndex < 0)
			{
				return;
			}

			handler(ctx);
		});
	}
}

static int32_t* g_maxBestSpawnPoints;
static void FixNatives()
{
	{
		constexpr const uint64_t nativeHash = 0xae51bc858f32ba66; // N_0xae51bc858f32ba66 (PROCGRASS_ENABLE_CULLSPHERE)

		const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			int32_t idx = ctx.GetArgument<uint32_t>(0);
			if (idx < 0 || idx >= 8)
			{
				return;
			}

			handler(ctx);
		});
	}

	{
		constexpr const uint64_t nativeHash = 0x6C34F1208B8923FD; // NETWORK_GET_RESPAWN_RESULT_FLAGS

		const auto handler = fx::ScriptEngine::GetNativeHandler(nativeHash);
		if (!handler)
		{
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(nativeHash, [handler](fx::ScriptContext& ctx)
		{
			int32_t idx = ctx.GetArgument<uint32_t>(0);
			if (idx < 0 || idx >= *g_maxBestSpawnPoints)
			{
				ctx.SetResult(0);
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
	g_trainConfigData = hook::get_address<rage::CTrainConfigData*>(hook::get_pattern<rage::CTrainConfigData>("4C 8B 05 ? ? ? ? 0F 29 74 24 ? 48 8D 3C 40", 3));
	g_maxBestSpawnPoints = hook::get_address<int32_t*>(hook::get_pattern<int32_t>("4C 63 05 ? ? ? ? 33 D2 4C 8B C9", 3));

	// Stolen from "VehicleExtraNatives.cpp"
	g_vehicleTypeOffset = *hook::get_pattern<int>("41 83 BF ? ? ? ? 0B 74", 3);

	void* breakOffMainRotorAddress = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B B1 ? ? ? ? 0F BE B9 ? ? ? ? 48 8B D9 48 85 F6 0F 84");
	CHeli_BreakOffMainRotor = reinterpret_cast<CHeli_BreakOffRotor_t>(breakOffMainRotorAddress);
	void* breakOffTailRotorAddress = hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B B1 ? ? ? ? 0F BE B9 ? ? ? ? 48 8B D9 48 85 F6 74");
	CHeli_BreakOffTailRotor = reinterpret_cast<CHeli_BreakOffRotor_t>(breakOffTailRotorAddress);

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		BlockForbiddenNatives();

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

		FixApplyForceToEntity();

		FixMissionTrain();

		FixAddRopeNative();

		FixSetHeliRotorHealth();

		FixSetPlayerParachuteModelOverride();
		FixSetPlayerParachutePackModelOverride();

		FixActionscriptFlagNatives();

		FixNatives();

		if (xbr::IsGameBuildOrGreater<2612>())
		{
			// IS_BIT_SET is missing in b2612+, re-adding for compatibility
			FixIsBitSet();
		}
	});
});
