#include <StdInc.h>

#include <CoreConsole.h>

#include <jitasm.h>
#include <Hooking.h>
#include <Hooking.Stubs.h>

static int32_t getHeapSize_offset = 0;
static int32_t getMemoryUsed_offset = 0;
static int32_t getMemoryAvailable_offset = 0;
static int32_t memAllocIdOffset = 0;
static int32_t heapIdBits = 0;

static size_t lastAllocationSize = 0;
static void*(*g_Allocate)(void* self, size_t size, size_t align);
static void* Allocate_Hook(void* self, size_t size, size_t align)
{
	lastAllocationSize = size;
	return g_Allocate(self, size, align);
}

static void OnAllocationFailed(hook::FlexStruct* self)
{
	size_t heapSize = self->CallVirtual<size_t>(getHeapSize_offset);
	size_t memoryUsed = self->CallVirtual<size_t, int32_t>(getMemoryUsed_offset, -1);
	size_t memoryAvailable = self->CallVirtual<size_t>(getMemoryAvailable_offset);

	const int heapIdMask = (1 << heapIdBits) - 1;
	const int heapId = self->Get<int>(memAllocIdOffset) & ~heapIdMask;

	trace("Error allocating %d bytes in heap with id %d (Total: %d, Used: %d, Free: %d)\n", lastAllocationSize, heapId, heapSize, memoryUsed, memoryAvailable);
}

static HookFunction hookFunction([]
{
	auto allocateLoc = hook::get_pattern<uint8_t>("48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC ? 48 8D B1 ? ? ? ? 49 8B E8");
	g_Allocate = hook::trampoline(allocateLoc, Allocate_Hook);

	auto onAllocationFailed = hook::get_pattern<uint8_t>("FF 90 ? ? ? ? 48 8B 03 83 CA");

	getHeapSize_offset = *(uint32_t*)(onAllocationFailed + 2);
	getMemoryUsed_offset = *(uint32_t*)(onAllocationFailed + 17);
	getMemoryAvailable_offset = *(uint32_t*)(onAllocationFailed + 29);

	auto loc = hook::get_pattern<int8_t>("C1 E7 ? FF C7 33 D2");
	heapIdBits = *(int8_t*)(loc + 2);
	memAllocIdOffset = *(int32_t*)(loc + 16);

	hook::nop(onAllocationFailed, 33);
	hook::call(onAllocationFailed, OnAllocationFailed);
});
