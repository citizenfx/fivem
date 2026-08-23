#include <StdInc.h>

#include <Hooking.h>
#include <MinHook.h>

#include "PlayerPatches.h"

static HookFunction hookFunction([]()
{
	ApplyPlayerDamageArrayPatches();
	ApplyAnimSceneHandlerPatches();
	ApplyAnimSceneArrayHandlerPatches();
	ApplyPlayerCachePatches();
	ApplyPlayerBandwidthPatches();
	ApplyPlayerDamageTrackerPatches();
	ApplyNetObjectPlayerIndexPatches();
	ApplyNetPlayerMgrPatches();
	ApplyPlayerIndexComparisons();
	ApplyPlayerIterationPatches();
	ApplyPedIntelligencePatches();
	ApplyPlayerStackResizes();
	ApplyBubbleJoinPatch();
	ApplyPlayerBitsetPatches();
	ApplyPlayerSyncDataPatch();
	ApplyNodeDataSentPatch();
	ApplySyncDataOwnerPatch();
	ApplyScriptHandlerNodePatch();
	ApplyGhostMaskPatch();
	ApplyRemoteScriptInfoPatch();
	ApplyPlayerMgrFlagsPatch();
	ApplyScriptHandlerBitsetPatches();
	ApplyCachedPlayerDataPatch();
	ApplySyncMessageSequencePatch();
	ApplyDisabledSubsystemPatches();
	ApplyNetworkObjectMgrPatches();

	MH_Initialize();

	CreateDisabledSubsystemHooks();
	CreatePlayerFocusHooks();
	CreatePlayerBitsetHooks();
	CreatePlayersNearPointHook();
	CreateDisabledTelemetryHooks();

	MH_EnableHook(MH_ALL_HOOKS);
});
