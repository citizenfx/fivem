#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

namespace rage 
{
	class phBoundComposite
	{
	public:
		char m_Padding[0xA0];
		uint16_t m_MaxNumBounds;
		uint16_t m_NumBounds;
	};

	static_assert(sizeof(phBoundComposite) == 0xA4, "rage::phBoundComposite has wrong size!");
}

static uint32_t g_RagdollInstanceOffset = 0x0;

static void (*g_CTaskNMShot_ClampHitToSurfaceOfBound)(hook::FlexStruct*, hook::FlexStruct*, hook::FlexStruct*, uint32_t);

static void CTaskNMShot_ClampHitToSurfaceOfBound(hook::FlexStruct* thisPtr, hook::FlexStruct* ped, hook::FlexStruct* localPos, uint32_t component)
{
	// NOTE: already checked in all builds, offsets dont need a signature
	auto* compositeBound = ped->Get<hook::FlexStruct*>(g_RagdollInstanceOffset)
		->Get<hook::FlexStruct*>(0x68)
		->Get<rage::phBoundComposite*>(0x108);

	if (component > compositeBound->m_NumBounds)
	{
		component = compositeBound->m_NumBounds;
	}

	g_CTaskNMShot_ClampHitToSurfaceOfBound(thisPtr, ped, localPos, component);
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

	g_RagdollInstanceOffset = *hook::get_pattern<uint32_t>("48 8B 82 ? ? ? ? 49 8B D8 4C 8B DA", 3);

	g_CTaskNMShot_ClampHitToSurfaceOfBound = hook::trampoline(hook::get_pattern("48 8B C4 48 89 58 ? 48 89 78 ? 55 48 8D 68 ? 48 81 EC ? ? ? ? 0F 29 70 ? 0F 29 78 ? 44 0F 29 40 ? 48 8B 82"), CTaskNMShot_ClampHitToSurfaceOfBound);
});
