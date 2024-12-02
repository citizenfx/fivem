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
#include "RageParser.h"
#include "ScriptWarnings.h"

#include <vector>

static bool* g_textCentre;
static bool* g_textDropshadow;


static void BlockForbiddenNatives()
{
	std::vector<uint64_t> nativesToBlock = rage::scrEngine::GetBlockedNatives();
	for (auto native: nativesToBlock)
	{
		fx::ScriptEngine::RegisterNativeHandler(native, [](fx::ScriptContext& ctx)
		{
			ctx.SetResult<uintptr_t>(0);
		});
	}
}

static void RedirectNoppedTextNatives()
{
	// original hash, target hash
	std::pair<uint64_t, uint64_t> nativesRedirects[] = {
		{ 0xD79334A4BB99BAD1, 0x16794E044C9EFB58 }, // _DISPLAY_TEXT to _BG_DISPLAY_TEXT
		{ 0x50A41AD966910F03, 0x16FA5CE47F184F1E }, // _SET_TEXT_COLOR to _BG_SET_TEXT_COLOR
		{ 0x4170B650590B3B00, 0xA1253A3C870B6843 }, // _SET_TEXT_SCALE to _BG_SET_TEXT_SCALE
	};

	for (auto native : nativesRedirects)
	{
		auto targetHandler = fx::ScriptEngine::GetNativeHandler(native.second);

		if (!targetHandler)
		{
			trace("Couldn't find 0x%08x handler to hook!\n", native.second);
			return;
		}

		fx::ScriptEngine::RegisterNativeHandler(native.first, [=](fx::ScriptContext& ctx)
		{
			(*targetHandler)(ctx);
		});
	}
}

static void ImplementRemovedTextNatives()
{
	// SET_TEXT_CENTRE
	fx::ScriptEngine::RegisterNativeHandler(0xBE5261939FBECB8C, [=](fx::ScriptContext& ctx)
	{
		auto value = *g_textCentre;

		if (ctx.GetArgument<bool>(0))
		{
			value = 0;
		}

		*g_textCentre = value;
	});

	// SET_TEXT_DROPSHADOW
	fx::ScriptEngine::RegisterNativeHandler(0x1BE39DBAA7263CA5, [=](fx::ScriptContext& ctx)
	{
		*g_textDropshadow = (ctx.GetArgument<int>(0) > 0);
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

static HookFunction hookFunction([]()
{
	if (xbr::IsGameBuildOrGreater<1436>())
	{
		auto location = hook::get_pattern<char>("0F 28 05 ? ? ? ? 83 25 ? ? ? ? 00 83 25 ? ? ? ? 00");

		g_textCentre = hook::get_address<bool*>(location + 0x33) + 2;
		g_textDropshadow = hook::get_address<bool*>(location + 0x3B) + 1;
	}

	rage::scrEngine::OnScriptInit.Connect([]()
	{
		BlockForbiddenNatives();

		// R* removed some text related natives since RDR3 1436.25 build.
		// Redirecting original natives to their successors to keep cross build compatibility.
		// Also re-implementing entirely removed natives.
		if (xbr::IsGameBuildOrGreater<1436>())
		{
			RedirectNoppedTextNatives();
			ImplementRemovedTextNatives();
		}

		FixPedCombatAttributes();
	});
});
