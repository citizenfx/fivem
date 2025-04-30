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
#include <EntitySystem.h>
#include <vector>

static bool* g_textCentre;
static bool* g_textDropshadow;

struct netObject
{
	char pad[64]; // +0
	uint16_t objectType; // +64
	uint16_t objectId; // +66
	char pad2[1]; // +68
	uint8_t ownerId; // +69
	uint8_t nextOwnerId; // +70
	bool isRemote; // +71
	char pad3[16]; // +72
	fwEntity* gameObject; // +88
};

static bool CanBlendWhenFixed(void* a1)
{
	netObject* netObj = (netObject*)(a1);

	return netObj->isRemote;
}

static void BlockForbiddenNatives()
{
	std::vector<uint64_t> nativesToBlock = rage::scrEngine::GetBlockedNatives();
	for (auto native : nativesToBlock)
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
			targetHandler(ctx);
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

static HookFunction hookFunction([]()
{
	{
		// Locate the vtable for CNetObjObject and override its blend function.
		// The function pointer at index 204 was originally a nullsub (returning 0).
		// We replace it with our override function to ensure blending always occurs when enabled.
		auto vtable = hook::get_address<uintptr_t*>(hook::get_pattern("48 8B F1 8B 84 ? ? ? ? ? 89 44 ? ? 8A 84 ? ? ? ? ? 88 44 ? ? E8 ? ? ? ? 33 DB", 35));
		hook::put<uintptr_t>(&vtable[204], (uintptr_t)CanBlendWhenFixed);
	}

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

	// GET_CLOSEST_OBJECT_OF_TYPE: avoid entities forced to be networked
	{
		// By default this native check if the entity is networked, if not it will make the entity networked and we don't want this native to change the network state of the entity to have the same behavior as in fivem
		auto location = hook::get_pattern("48 85 C0 74 ? 40 38 78 ? 74 ? 8B 15");
		hook::nop(location, 31);
	}
	
	{
		// Clears the "not visible" flags when disabling the conceal state (NETWORK_CONCEAL_PLAYER) for the player.
		// This is necessary to make the player visible again after disabling the conceal state because, 
		// before showing the entity again, the game checks that it does not have any "not visible" flags. 
		// By default, the game does not clear these flags when executing this native function.
		static struct : jitasm::Frontend
		{
			intptr_t location;
			void Init(intptr_t location)
			{
				this->location = location + 10;
			}
			
			void InternalMain() override
			{
				mov(r8b, 01);
				lea(rdi, qword_ptr[r14+0x270]);
				mov(rsi, qword_ptr[rdi]);
				
				and(dword_ptr[rdi+0x30], 0); // m_remoteVisibilityOverride->m_isNotVisibleFlags = 0;
			
				mov(r15, location);
				jmp(r15); // Jump back to the original code
			}
		} patchStub;
		auto location = hook::get_pattern("41 B0 ? 49 8D BE");
		hook::nop(location, 13);
		patchStub.Init(reinterpret_cast<intptr_t>(location));
		hook::jump(location, patchStub.GetCode());
		
		// mov rax, [r14] (restore rax value from r14)
		hook::put((char*)location + 10, 0x068B49);
	}
});
