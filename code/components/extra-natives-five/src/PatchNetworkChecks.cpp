/*
* This file is part of the CitizenFX project - http://citizen.re/
*
* See LICENSE and MENTIONS in the root of the source tree for information
* regarding licensing.
*/

#include "StdInc.h"
#include <Hooking.h>
#include <nutsnbolts.h>
#include <GameInit.h>
#include <CoreConsole.h>

static bool* g_flyThroughWindscreenDisabled;
static bool* g_playerRagdollOnCollisionDisabled;
static bool* g_playerJumpRagdollControlDisabled;
static bool* g_dynamicDoorCreationDisabled;

static bool isFlyThroughWindscreenEnabledConVar = false;
static bool isPlayerRagdollOnCollisionEnabledConVar = false;
static bool isPlayerJumpRagdollControlEnabledConVar = false;
static bool isDynamicDoorCreationEnabledConVar = false;

static bool* isNetworkGame;

static HookFunction hookFunction([]()
{
	isNetworkGame = hook::get_address<bool*>(hook::get_pattern("24 07 3C 03 75 12 40 38 35 ? ? ? ? 75 09 83", 9));

	static ConVar<bool> enableFlyThroughWindscreen("game_enableFlyThroughWindscreen", ConVar_Replicated, false, &isFlyThroughWindscreenEnabledConVar);
	static ConVar<bool> enablePlayerRagdollOnCollision("game_enablePlayerRagdollOnCollision", ConVar_Replicated, false, &isPlayerRagdollOnCollisionEnabledConVar);
	static ConVar<bool> enablePlayerJumpRagdollControl("game_enablePlayerJumpRagdollControl", ConVar_Replicated, false, &isPlayerJumpRagdollControlEnabledConVar);
	static ConVar<bool> enableDynamicDoorCreation("game_enableDynamicDoorCreation", ConVar_Replicated, false, &isDynamicDoorCreationEnabledConVar);
	
	// replace netgame check for fly through windscreen with our variable
	{
		g_flyThroughWindscreenDisabled = (bool*)hook::AllocateStubMemory(1);
		auto location = hook::get_pattern<uint32_t>("45 33 ED 44 38 2D ? ? ? ? 4D", 6);
		hook::put<int32_t>(location, (intptr_t)g_flyThroughWindscreenDisabled - (intptr_t)location - 4);
	}

	// replace netgame check for ragdoll on collision with our variable
	{
		g_playerRagdollOnCollisionDisabled = (bool*)hook::AllocateStubMemory(1);
		auto location = hook::get_pattern<uint32_t>("0F 45 C8 84 C9 0F 84", -18);
		hook::put<int32_t>(location, (intptr_t)g_playerRagdollOnCollisionDisabled - (intptr_t)location - 4);
	}

	// replace netgame check for jump ragdoll control with our variable
	{
		g_playerJumpRagdollControlDisabled = (bool*)hook::AllocateStubMemory(1);
		auto location = hook::get_pattern<uint32_t>("C1 E8 03 A8 01 0F 85 ? ? ? ? F6 83 ? ? ? ? 80", -17);
		hook::put<int32_t>(location, (intptr_t)g_playerJumpRagdollControlDisabled - (intptr_t)location - 4 - 1);
	}

	// replace netgame check for dynamic door creation with our variable
	{
		g_dynamicDoorCreationDisabled = (bool*)hook::AllocateStubMemory(1);
		void* location = nullptr;
		if (xbr::IsGameBuildOrGreater<3258>())
		{
			location = hook::get_pattern<uint32_t>("44 8A 15 ? ? ? ? C1 E8", 3);
		}
		else
		{
			location = hook::get_pattern<uint32_t>("44 8A 15 ? ? ? ? C1 E8 13 A8 01 74 0E", 3);
		}
		hook::put<int32_t>(location, (intptr_t)g_dynamicDoorCreationDisabled - (intptr_t)location - 4);
	}

	OnMainGameFrame.Connect([]()
	{
		*g_flyThroughWindscreenDisabled = *isNetworkGame && !isFlyThroughWindscreenEnabledConVar;
		*g_playerRagdollOnCollisionDisabled = *isNetworkGame && !isPlayerRagdollOnCollisionEnabledConVar;
		*g_playerJumpRagdollControlDisabled = *isNetworkGame && !isPlayerJumpRagdollControlEnabledConVar;
		*g_dynamicDoorCreationDisabled = *isNetworkGame && !isDynamicDoorCreationEnabledConVar;
	});
});
