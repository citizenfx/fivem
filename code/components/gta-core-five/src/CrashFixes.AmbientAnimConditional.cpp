#include <StdInc.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

static int32_t allocationSize = 0;
static int32_t allocationAlign = 0;
static void* (*g_allocateTask)(void* allocator, uint64_t size, uint64_t align);
static void (*g_quitCritical)(uint32_t errorHash);

static void* allocator = nullptr;

void* (*g_CTaskAmbientClipsCtor)(void* memAllocated, uintptr_t unk1, uintptr_t condGroupPtr, uintptr_t unk2, uintptr_t unk3);
void* CTaskAmbientClipsCtor(void* memAllocated, uintptr_t unk1, uintptr_t condGroupPtr, uintptr_t unk2, uintptr_t unk3)
{
	if (!condGroupPtr)
	{
		return nullptr;
	}

	void* allocatedMemory = g_allocateTask(*(void**)allocator, allocationSize, allocationAlign);
	if (!allocatedMemory)
	{
		g_quitCritical(HashString("ERR_MEM_POOLALLOC_ALLOC_2"));
		return nullptr;
	}
	
	return g_CTaskAmbientClipsCtor(allocatedMemory, unk1, condGroupPtr, unk2, unk3);
}

static HookFunction hookFunction([]
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
});
