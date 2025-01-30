#include "StdInc.h"
#include "Hooking.h"

#include <CrossBuildRuntime.h>

static void*(*g_origMemAlloc)(void*, intptr_t size, intptr_t align, int subAlloc);
static intptr_t(*g_origMemFree)(void*, void*);
static bool(*g_origIsMine)(void*, void*);
static bool(*g_origRealloc)(void*, void*, size_t);

static bool isMine(void* allocator, void* mem)
{
	return (*(uint32_t*)((DWORD_PTR)mem - 4) & 0xFFFFFFF0) == 0xDEADC0C0;
}

static bool isMineHook(void* allocator, void* mem)
{
	return isMine(allocator, mem) || g_origIsMine(allocator, mem);
}

static void* AllocEntry(void* allocator, size_t size, int align, int subAlloc)
{
	DWORD_PTR ptr = (DWORD_PTR)malloc(size + 32);
	ptr += 4;

	void* mem = (void*)(((uintptr_t)ptr + 15) & ~(uintptr_t)0xF);

	*(uint32_t*)((uintptr_t)mem - 4) = 0xDEADC0C0 | (((uintptr_t)ptr + 15) & 0xF);

	return mem;
}

static void FreeEntry(void* allocator, void* ptr)
{
	if (!isMine(allocator, ptr))
	{
		g_origMemFree(allocator, ptr);
		return;
	}

	void* memReal = ((char*)ptr - (16 - (*(uint32_t*)((uintptr_t)ptr - 4) & 0xF)) - 3);
	free(memReal);
}

static void ReallocEntry(void* allocator, void* ptr, size_t size)
{
	if (g_origIsMine(allocator, ptr))
	{
		g_origRealloc(allocator, ptr, size);
		return;
	}

	return;
}

static void* smpaCtor;

static void* CreateSimpleAllocatorHook(void* a1, void* a2, void* a3, int a4, int a5)
{
	void* smpa = ((void*(*)(void*, void*, void*, int, int))smpaCtor)(a1, a2, a3, a4, a5);

	// 2802 RTTI changes move desired functions down by 6 entries
	if (xbr::IsGameBuildOrGreater<2802>())
	{
		void** vt = new void*[49];
		memcpy(vt, *(void**)smpa, 49 * 8);
		*(void**)smpa = vt;

		g_origMemAlloc = (decltype(g_origMemAlloc))vt[8];
		vt[8] = AllocEntry;

		g_origMemFree = (decltype(g_origMemFree))vt[10];
		vt[10] = FreeEntry;

		g_origRealloc = (decltype(g_origRealloc))vt[12];
		vt[12] = ReallocEntry;

		g_origIsMine = (decltype(g_origIsMine))vt[32];
		vt[32] = isMineHook;
	}
	else
	{
		void** vt = new void*[45];
		memcpy(vt, *(void**)smpa, 45 * 8);
		*(void**)smpa = vt;

		g_origMemAlloc = (decltype(g_origMemAlloc))vt[2];
		vt[2] = AllocEntry;

		g_origMemFree = (decltype(g_origMemFree))vt[4];
		vt[4] = FreeEntry;

		g_origRealloc = (decltype(g_origRealloc))vt[6];
		vt[6] = ReallocEntry;

		g_origIsMine = (decltype(g_origIsMine))vt[26];
		vt[26] = isMineHook;
	}

	return smpa;
}

static HookFunction hookFunction([]()
{
	smpaCtor = hook::get_pattern("41 8B F9 48 89 01 49 63 F0 48 8B EA B9 07", -0x23);
	hook::call(hook::get_pattern("44 8D 49 04 88 4C 24 20 44 8B ? 48 8B", 0x11), CreateSimpleAllocatorHook);

	// allocation of fragment bone cache
	auto loc = hook::get_pattern("48 89 1F 0F B7 5B 5E B8 40 00 00 00", 8);
	hook::put<uint32_t>(loc, *(uint32_t*)loc * 8);
});
