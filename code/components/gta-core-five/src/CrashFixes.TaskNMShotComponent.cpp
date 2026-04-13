#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

enum eRagdollComponent
{
	RAGDOLL_BUTT = 0,

	RAGDOLL_UPPER_LEG_LEFT,
	RAGDOLL_LOWER_LEG_LEFT,
	RAGDOLL_LEFT_FOOT,
	RAGDOLL_UPPER_LEG_RIGHT,
	RAGDOLL_LOWER_LEG_RIGHT,
	RAGDOLL_RIGHT_FOOT,
	RAGDOLL_LOWER_BACK,
	RAGDOLL_MID_BACK,
	RAGDOLL_UPPER_BACK,
	RAGDOLL_UPPER_CHEST,
	RAGDOLL_LEFT_SHOULDER,
	RAGDOLL_LEFT_UPPER_ARM,
	RAGDOLL_LEFT_LOWER_ARM,
	RAGDOLL_LEFT_HAND,
	RAGDOLL_RIGHT_SHOULDER,
	RAGDOLL_RIGHT_UPPER_ARM,
	RAGDOLL_RIGHT_LOWER_ARM,
	RAGDOLL_RIGHT_HAND,
	RAGDOLL_NECK,
	RAGDOLL_HEAD,

	RAGDOLL_MAX_COMPONENTS
};

static void (*g_CClonedNMShotInfo_Serialise)(hook::FlexStruct*, hook::FlexStruct*);

static void CClonedNMShotInfo_Serialise(hook::FlexStruct* thisPtr, hook::FlexStruct* serialiser)
{
	g_CClonedNMShotInfo_Serialise(thisPtr, serialiser);

	uint8_t& component = thisPtr->At<uint8_t>(0x68);

	if (component >= RAGDOLL_MAX_COMPONENTS)
	{
		component = RAGDOLL_HEAD;
	}
}

static HookFunction hookFunction([]()
{
	// A null pointer dereference can occur during the NM Shot task, which is for ragdoll-inducing weapons like stun gun.
	//
	// Cheaters can serialize an out-of-bounds m_Component value,
	// the game later uses this invalid index when accessing phBoundComposite bounds data.
	// This can result in a nullptr dereference, crashing the client.
	//
	// This hook clamps the component index to the valid range preventing out-of-bounds access and avoiding the crash.

	g_CClonedNMShotInfo_Serialise = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8B FA 48 8B F1 E8 ? ? ? ? ? ? ? 48 8D 56 ? 45 33 C0 48 8B CF FF 50 ? 80 7E ? 00 75 ? ? ? ? 48 8B CF FF 90 ? ? ? ? 84 C0 74 ? ? ? ? 48 8D 56"), CClonedNMShotInfo_Serialise);
});
