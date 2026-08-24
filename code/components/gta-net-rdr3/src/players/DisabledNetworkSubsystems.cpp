#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>
#include <MinHook.h>

#include <GameInit.h>

extern ICoreGameInit* icgi;

template<bool Onesync, bool Legacy>
static bool Return()
{
	return icgi->OneSyncEnabled ? Onesync : Legacy;
}

static uint8_t (*g_origunkNetworkObjectMgr__AccessObjects)();
static uint8_t unkNetworkObjectMgr__AccessObjects()
{
	if (!icgi->OneSyncEnabled)
	{
		return g_origunkNetworkObjectMgr__AccessObjects();
	}

	return 0;
}

static void (*g_unkRemoteBroadcast)(void*, __int64);
static void unkRemoteBroadcast(void* a1, __int64 a2)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkRemoteBroadcast(a1, a2);
	}
}

static void* (*g_unkP2PObjectInit)(void*);
static void* unkP2PObjectInit(void* objectMgr)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkP2PObjectInit(objectMgr);
	}
	return nullptr;
}

static int* sub_1424(void* a1, int* a2, void* a3, bool a4)
{
	*a2 = 0;
	return a2;
}

static unsigned long (*g_netArrayManager__Update)(void*);
static unsigned long netArrayManager__Update(void* a1)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_netArrayManager__Update(a1);
	}
	return 0;
}

static void* (*g_unkBandwidthTelemetry)(void*, int);
static void* unkBandwidthTelemetry(void* bandwidthMgr, int a2)
{
	if (!icgi->OneSyncEnabled)
	{
		return g_unkBandwidthTelemetry(bandwidthMgr, a2);
	}

	return nullptr;
}

static void ApplyDisabledSubsystemPatches()
{
	// Skip unused host kick related >32-unsafe arrays in onesync
	hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 75 ? 8B 05 ? ? ? ? 33 C9 89 44 24"), Return<true, false>);
	hook::call(hook::get_pattern("E8 ? ? ? ? 84 C0 75 ? 80 7B ? ? 73 ? 48 8B CB"), Return<true, false>);

	// nop "Last Too Many Objects ACK" network log, indexes an 32 sized array without a < 32 check
	{
		auto location = hook::get_pattern("4C 8D 05 ? ? ? ? 48 8D 15 ? ? ? ? 4C 8B 10 45 8B 8C 8E");
		hook::nop(location, 32);
	}

	// Remove network text chat. Unused in RedM/RDR but uses 32 sized arrays with minimal checks
	{
		// Update
		hook::nop(hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 45 84 F6 74", -23), 28);

		// AddPlayer
		hook::nop(hook::get_pattern("48 8B 0D ? ? ? ? E8 ? ? ? ? 84 C0 74 ? 48 8B 0D ? ? ? ? 44 8B C7"), 34);
	}

	// Temporarily disable network voice chat. RDR3's implementation slightly differs from GTA V's and requires further investigation as it has several 32-sized arrays
	{
		hook::return_function(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 7B ? 48 8B CF E8 ? ? ? ? 84 C0")));

		hook::return_function(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 68 ? 48 89 70 ? 48 89 78 ? 41 54 41 56 41 57 48 83 EC ? 48 8B D9 E8 ? ? ? ? 8B F0"));
	}

	// Temporary, CRespawnPlayerPedEvent has a int32 bitset with no >32 check, the event will need to be reworked in the future as it depends on this bitset
	// otherwise some unexpected behaviour with leftover ped can occur. But good enough for testing.
	hook::nop(hook::get_pattern("8B 44 86 ? 0F A3 C8 73 ? 0F B7 56"), 7);

	//TEMPORARY: this has no >32 checks and writes into two bitsets located in CNetObjPlayer with critical data next to it. you can probably imagine what happens...
	//In onesync this logic is redundant. But still gets called in player respawn events replies.
	hook::return_function(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B 01 49 8B D8 40 8A EA 48 8B F1"));

	// Unsafe >32 out of bound reads to player objects data, which isn't used (or populated) in onesync.
	{
		auto location = hook::get_pattern("E8 ? ? ? ? E8 ? ? ? ? 80 A7 ? ? ? ? ? C0 E0");
		hook::set_call(&g_origunkNetworkObjectMgr__AccessObjects, location);
		hook::call(location, unkNetworkObjectMgr__AccessObjects);
	}
}

static void CreateDisabledSubsystemHooks()
{
	// Don't broadcast script info for script created vehicles in OneSync.
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 55 48 8B EC 48 81 EC ? ? ? ? 48 83 79"), unkRemoteBroadcast, (void**)&g_unkRemoteBroadcast);
	// Don't call init related object iteration (32-sized player object array with no >32 check)
	MH_CreateHook(hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8B 0D ? ? ? ? BA ? ? ? ? E8 ? ? ? ? 48 8B 0D ? ? ? ? E8 ? ? ? ? 48 8D 0D")), unkP2PObjectInit, (void**)&g_unkP2PObjectInit);
	MH_CreateHook(hook::get_pattern("48 8B 0F 0F B6 51 19 48 03 D2 49 8B 5C D6 08", -49), unkP2PObjectInit, NULL);
}

static void CreateDisabledTelemetryHooks()
{
	//TEMP: Potentially can overflow and lead to issues, and this logic isn't important in onesync at the moment.
	MH_CreateHook(hook::get_pattern("48 89 5C 24 ? 55 56 57 41 54 41 55 41 56 41 57 48 83 EC ? 65 48 8B 0C 25 ? ? ? ? 4C 8B F2"), sub_1424, NULL);
	MH_CreateHook(hook::get_pattern("48 89 4C 24 ? 53 55 56 57 41 54 41 55 41 56 41 57 B8 ? ? ? ? E8 ? ? ? ? 48 2B E0 48 8B F9"), netArrayManager__Update, (void**)&g_netArrayManager__Update);
	MH_CreateHook(hook::get_pattern("40 55 53 56 57 41 54 41 55 41 56 41 57 48 8D 6C 24 ? 48 81 EC ? ? ? ? 33 F6"), unkBandwidthTelemetry, (void**)&g_unkBandwidthTelemetry);
}

static HookFunction hookFunction([]()
{
	ApplyDisabledSubsystemPatches();

	MH_Initialize();

	CreateDisabledSubsystemHooks();
	CreateDisabledTelemetryHooks();

	MH_EnableHook(MH_ALL_HOOKS);
});
