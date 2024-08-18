#include <StdInc.h>

#include <Hooking.h>
#include <Hooking.Stubs.h>

static hook::cdecl_stub<bool(void*, void*)> CPhysical__IsParentAttachment([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 84 C0 0F 85 ? ? ? ? 41 83 CD"));
});

bool (*g_origCPhysical__AttachUnk)(void* physical, void* otherPhysical, int16_t unk0, int16_t unk1, int32_t unk2, uintptr_t unk3, uintptr_t unk4, uintptr_t unk5, float unk6, bool unk7, float unk8, float unk9);
bool CPhysical__AttachUnk(void* physical, void* otherPhysical, int16_t unk0, int16_t unk1, int32_t unk2, uintptr_t unk3, uintptr_t unk4, uintptr_t unk5, float unk6, bool unk7, float unk8, float unk9)
{
	if (otherPhysical && CPhysical__IsParentAttachment(otherPhysical, physical))
	{
		trace("Stack overflow crash prevented [CFX-1734]\n");

		return false;
	}

	return g_origCPhysical__AttachUnk(physical, otherPhysical, unk0, unk1, unk2, unk3, unk4, unk5, unk6, unk7, unk8, unk9);
}

static HookFunction hookFunction([]
{
	g_origCPhysical__AttachUnk = hook::trampoline(hook::get_pattern<void>("45 0F B7 F9 45 0F B7 E0 48 8B F2", -0x36), &CPhysical__AttachUnk);
});
