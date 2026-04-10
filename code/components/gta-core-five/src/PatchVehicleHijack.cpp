#include <StdInc.h>

#include <CoreConsole.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

#include <jitasm.h>

#include <bitset>

static bool g_EnableVehicleHijackFix = false;

struct CSyncedPed
{
	uint16_t m_Id;
	uint32_t m_EntityID;
};

enum CVehicleJackBitFlags
{
	JackIfOccupied = 3,
	BeJacked = 11,
	JustPullPedOut = 19,
	DontJackAnyone = 23,
	Unk = 31,
	JackedFromInside = 33,
	JackAndGetIn = 44,
	JackWantedPlayersRatherThanStealCar = 46,
	GettingOffBecauseDriverJacked = 52,
	ConsiderJackingFriendlyPeds = 60,
	WantsToJackFriendlyPed = 61,
	HasCheckedForFriendlyJack = 62,
	ForceFleeAfterJack = 65,
	AllowBlockedJackReactions = 67,
	WaitForJackInterrupt = 68,
	UseOnVehicleJack = 69,
	HasJackedAPed = 71,
	SwitchedPlacesWithJacker = 72
};

static constexpr size_t kVehicleEnterExitFlagCount = 96;
using CVehicleEnterExitFlags = std::bitset<kVehicleEnterExitFlagCount>;

static uint8_t g_EnterExitFlagOffset = 0x0;
static uint8_t g_SyncedPedOffset = 0x0;

static void (*g_CClonedEnterVehicleInfo_Serialise)(hook::FlexStruct*, hook::FlexStruct*);

static void CClonedEnterVehicleInfo_Serialise(hook::FlexStruct* clonedVehicleFsmInfo, hook::FlexStruct* serialiser)
{
	g_CClonedEnterVehicleInfo_Serialise(clonedVehicleFsmInfo, serialiser);

	if (g_EnableVehicleHijackFix)
	{
		CVehicleEnterExitFlags& enterExitFlags = clonedVehicleFsmInfo->At<CVehicleEnterExitFlags>(g_EnterExitFlagOffset);

		enterExitFlags.reset(JustPullPedOut);
		enterExitFlags.reset(JackIfOccupied);
		enterExitFlags.reset(JackAndGetIn);
		enterExitFlags.reset(HasJackedAPed);

		CSyncedPed& jackedPed = clonedVehicleFsmInfo->At<CSyncedPed>(0x60);
		jackedPed.m_Id = 0;
		jackedPed.m_EntityID = 0;
	}
}

enum FSM_Return : int32_t {
	Continue,
	Return
};

static FSM_Return (*g_CTaskEnterVehicle_UpdateClonedFSM)(hook::FlexStruct*, uint32_t, uint32_t);

static FSM_Return CTaskEnterVehicle_UpdateClonedFSM(hook::FlexStruct* thisPtr, uint32_t state, uint32_t event)
{
	if (g_EnableVehicleHijackFix && state == 0x15)
	{
		return FSM_Return::Return;
	}

	return g_CTaskEnterVehicle_UpdateClonedFSM(thisPtr, state, event);
}

static HookFunction hookFunction([]
{
	// NOTE: for servers that require vehicle hijacking, set this to false; however, since the number of cheaters hijacking vehicles exceeds the actual use case, this is set to true by default
	static ConVar<bool> enableVehicleHijackFix("game_enableVehicleHijackFix", ConVar_Replicated, true, &g_EnableVehicleHijackFix);

	g_EnterExitFlagOffset = *hook::get_pattern<uint8_t>("48 8D 53 ? 45 33 C0 48 8B CE E8 ? ? ? ? ? ? ? 45 33 C9", 3);
	g_SyncedPedOffset = *hook::get_pattern<uint8_t>("48 8D 4F ? ? ? ? ? 66 44 89 44 24", 3);

	g_CClonedEnterVehicleInfo_Serialise = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B F2 48 8B F9 E8 ? ? ? ? 48 8D 4F ? E8 ? ? ? ? 48 8D 54 24 ? 48 8D 4F"), CClonedEnterVehicleInfo_Serialise);
	g_CTaskEnterVehicle_UpdateClonedFSM = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 63 F2 41 8B F8"), CTaskEnterVehicle_UpdateClonedFSM);

	auto patchSetPedOutOfVehicleLocation = hook::get_pattern("41 83 C8 ? 48 8B C8 41 8D 50");
	auto patchSetPedOutOfVehicleSkipLocation = hook::get_pattern("41 B1 ? 44 8B C6 48 8B D3 48 8B CF C6 44 24");
	 
	static struct : jitasm::Frontend
	{
		uintptr_t m_ContinueAddress;
		uintptr_t m_SkipAddress;

		void Init(uintptr_t continueAddress, uintptr_t skipAddress)
		{
			m_ContinueAddress = continueAddress;
			m_SkipAddress = skipAddress;
		}

		void InternalMain() override
		{
			mov(r11, reinterpret_cast<uintptr_t>(&g_EnableVehicleHijackFix));
			cmp(byte_ptr[r11], 0);
			jne("SkipSetOutOfVehicle");

			or (r8d, -1);
			mov(rcx, rax);

			mov(r11, m_ContinueAddress);
			jmp(r11);

			L("SkipSetOutOfVehicle");
			mov(r11, m_SkipAddress);
			jmp(r11);
		}
	} patchStub;

	patchStub.Init((uintptr_t)patchSetPedOutOfVehicleLocation + 7, (uintptr_t)patchSetPedOutOfVehicleSkipLocation);

	hook::nop(patchSetPedOutOfVehicleLocation, 7);
	hook::jump_rcx(patchSetPedOutOfVehicleLocation, patchStub.GetCode());
});
