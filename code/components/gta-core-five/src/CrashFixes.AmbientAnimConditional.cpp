#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

#include "CrossBuildRuntime.h"

static int32_t allocationSize = 0;
static int32_t allocationAlign = 0;
static void* (*g_allocateTask)(void* allocator, uint64_t size, uint64_t align);
static void (*g_quitCritical)(uint32_t errorHash);
static void* allocator = nullptr;

struct atArray_CConditionalAnims
{
	void* m_Elements;
	uint16_t m_Count;
	uint16_t m_Capacity;
	uint16_t m_pad1;
	uint16_t m_pad2;
};
static_assert(sizeof(atArray_CConditionalAnims) == 0x10, "atArray_CConditionalAnims has wrong size!");

struct CConditionalAnimsGroup
{
	uint32_t m_hashname;
	atArray_CConditionalAnims m_conditionalAnims;
};
static_assert(sizeof(CConditionalAnimsGroup) == 0x18, "CConditionalAnimsGroup has wrong size!");

void* (*g_CTaskAmbientClipsCtor)(void* memAllocated, uintptr_t unk1, CConditionalAnimsGroup* condGroup, int condAnimChosen, uintptr_t unk3);
void* CTaskAmbientClipsCtor(void* memAllocated, uintptr_t unk1, CConditionalAnimsGroup* condGroup, int condAnimChosen, uintptr_t unk3)
{
	if (!condGroup)
	{
		return nullptr;
	}

	if (condAnimChosen >= condGroup->m_conditionalAnims.m_Count || condAnimChosen < -1)
	{
		return nullptr;
	}

	void* allocatedMemory = g_allocateTask(*(void**)allocator, allocationSize, allocationAlign);
	if (!allocatedMemory)
	{
		g_quitCritical(HashString("ERR_MEM_POOLALLOC_ALLOC_2"));
		return nullptr;
	}

	return g_CTaskAmbientClipsCtor(allocatedMemory, unk1, condGroup, condAnimChosen, unk3);
}

static hook::cdecl_stub<void*(uint32_t*)> fwClipSetManager_GetClipSet([]()
{
	return hook::get_call(hook::get_pattern("E8 ? ? ? ? 48 8D 54 24 ? 48 8B C8 E8 ? ? ? ? 8B 10"));
});

void (*g_CAmbientClipRequestHelper_SetClipAndProp)(void* thisPtr, uint32_t* clipSetId, uint32_t propHash);

void CAmbientClipRequestHelper_SetClipAndProp(void* thisPtr, uint32_t* clipSetId, const uint32_t propHash)
{
	if (!thisPtr)
		return;

	if (!clipSetId)
		return;

    if (const auto clipSet = fwClipSetManager_GetClipSet(clipSetId); !clipSet)
		return;

	g_CAmbientClipRequestHelper_SetClipAndProp(thisPtr, clipSetId, propHash);
}

static HookFunction hookFunction([]
{
	if (xbr::IsGameBuildOrGreater<3407>())
	{
		auto createCloneFsmAlloc = hook::get_pattern<uint8_t>("48 8B 0D ? ? ? ? BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 48 8B C8 48 85 C0 75 ? B9 ? ? ? ? E8 ? ? ? ? EB ? 8A 45 ? 44 8B 4D ? 8B 55");

		allocator = *(int32_t*)(createCloneFsmAlloc + 3) + createCloneFsmAlloc + 7;
		allocationSize = *(int32_t*)(createCloneFsmAlloc + 8);
		allocationAlign = *(int32_t*)(createCloneFsmAlloc + 14);
		g_allocateTask = (decltype(g_allocateTask))hook::get_call(createCloneFsmAlloc + 18);
		g_quitCritical = (decltype(g_quitCritical))hook::get_call(createCloneFsmAlloc + 36);

		// Completely removing memory allocation here
		hook::nop(createCloneFsmAlloc, 43);

		// Proxying constructor call and doing allocation right before ctor if allocation needed
		g_CTaskAmbientClipsCtor = (decltype(g_CTaskAmbientClipsCtor))hook::get_call(createCloneFsmAlloc + 65);
		hook::call(createCloneFsmAlloc + 65, &CTaskAmbientClipsCtor);
	}
	else
	{
		auto createCloneFsmAlloc = hook::get_pattern<uint8_t>("48 8B 0D ? ? ? ? BA ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ? 48 85 C0 75 ? B9 ? ? ? ? E8 ? ? ? ? EB ? 44 8B 4D");

		allocator = *(int32_t*)(createCloneFsmAlloc + 3) + createCloneFsmAlloc + 7;
		allocationSize = *(int32_t*)(createCloneFsmAlloc + 8);
		allocationAlign = *(int32_t*)(createCloneFsmAlloc + 14);
		g_allocateTask = (decltype(g_allocateTask))hook::get_call(createCloneFsmAlloc + 18);
		g_quitCritical = (decltype(g_quitCritical))hook::get_call(createCloneFsmAlloc + 33);

		// Completely removing memory allocation here
		hook::nop(createCloneFsmAlloc, 40);

		// Proxying constructor call and doing allocation right before ctor if allocation needed
		g_CTaskAmbientClipsCtor = (decltype(g_CTaskAmbientClipsCtor))hook::get_call(createCloneFsmAlloc + 58);
		hook::call(createCloneFsmAlloc + 58, &CTaskAmbientClipsCtor);
	}

	// Ambient clip info is serialized via cloned task data (e.g., CTaskWander).
	// A specific flag (1 << 20) overwrites the clip set ID with 0.

	// The game then immediately uses this invalid clip set ID in CAmbientClipRequestHelper::SetClipAndProp without validation.
	// fwClipSetManager::GetClipSet returns an invalid pointer, which is dereferenced when resolving the clip dictionary index,
	// causing an out-of-bounds access and client crash.

	g_CAmbientClipRequestHelper_SetClipAndProp = hook::trampoline(hook::get_pattern("48 89 5C 24 ? 48 89 74 24 ? 57 48 83 EC 20 41 8B F0 48 8B DA 48 8B F9 E8 ? ? ? ? 8B 03"), CAmbientClipRequestHelper_SetClipAndProp);
});
