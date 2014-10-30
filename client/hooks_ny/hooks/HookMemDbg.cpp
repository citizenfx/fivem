#include "StdInc.h"

#if 0
static HANDLE hHeap;

static void* __stdcall allocHook(size_t size, size_t align, int multiAllocator)
{
	DWORD ptr = (DWORD)malloc(size + 32);//HeapAlloc(hHeap, 0, size + 32);

	//return (void*)(ptr + 8);

	ptr += 4;

	void* mem = (void*)(((uintptr_t)ptr + 15) & ~(uintptr_t)0xF);

	*(uint32_t*)((uint32_t)mem - 4) = 0xDEADC0C0 | (((uintptr_t)ptr + 15) & 0xF);

	return mem;
}

DEFINE_INJECT_HOOK(heapAllocHook, 0x5A95A0)
{
	return JumpTo((DWORD)allocHook);
}

static void __stdcall freeHook(void* mem)
{
	void* memReal = ((char*)mem - (16 - (*(uint32_t*)((uint32_t)mem - 4) & 0xF)) - 3);

	//HeapFree(hHeap, 0, memReal);
	free(memReal);
}

DEFINE_INJECT_HOOK(heapFreeHook, 0x5A99C0)
{
	return JumpTo((DWORD)freeHook);
}

static bool __stdcall isMineHook(void* mem)
{
	return (*(uint32_t*)((DWORD)mem - 4) & 0xFFFFFFF0) == 0xDEADC0C0;
}

DEFINE_INJECT_HOOK(heapIsMineHook, 0x5A9CF0)
{
	return JumpTo((DWORD)isMineHook);
}

enum eCallPatcher
{
	PATCH_CALL,
	PATCH_JUMP,
	PATCH_NOTHING
};

void _patch(void* pAddress, DWORD data, DWORD iSize);
void _nop(void* pAddress, DWORD size);
void _call(void* pAddress, DWORD data, eCallPatcher bPatchType);

#define patch(a, v, s) _patch((void*)(a), (DWORD)(v), (s))
//#define nop(a, v) _nop((void*)(a), (v))
#define call(a, v, bAddCall) _call((void*)(a), (DWORD)(v), (bAddCall))

void ArrayHandlerDbgHook(void* a1, int a2, int a3, const char* format, ...)
{
	static char buffer[32768];
	va_list ap;
	va_start(ap, format);
	_vsnprintf(buffer, sizeof(buffer), format, ap);
	va_end(ap);

	trace("[debug] %s\n", buffer);
}

static HookFunction hookFunction([] ()
{
	hHeap = HeapCreate(0, 157286400, 0);

	// single-use mutex
	hook::nop(0x5AACB5, 2);
	hook::put<uint8_t>(0x5AACBC, 0xEB);

	// memory hooks
	call(0x5A95A0, allocHook, PATCH_JUMP);
	call(0x5A99C0, freeHook, PATCH_JUMP);
	call(0x5A9CF0, isMineHook, PATCH_JUMP);

	// test hooks
	call(0x493787, ArrayHandlerDbgHook, PATCH_CALL);
	call(0x4937B3, ArrayHandlerDbgHook, PATCH_CALL);
	call(0x4936AC, ArrayHandlerDbgHook, PATCH_CALL);
	call(0x493674, ArrayHandlerDbgHook, PATCH_CALL);
	call(0x493859, ArrayHandlerDbgHook, PATCH_CALL);
	/*heapAllocHook.inject();
	heapFreeHook.inject();
	heapIsMineHook.inject();*/
});

void _call(void* pAddress, DWORD data, eCallPatcher bPatchType)
{
	DWORD dwOldProtect;
	VirtualProtect(pAddress, 5, PAGE_EXECUTE_READWRITE, &dwOldProtect);

	switch (bPatchType)
	{
		case PATCH_JUMP:
			*(BYTE*)pAddress = (BYTE)0xE9;
			break;

		case PATCH_CALL:
			*(BYTE*)pAddress = (BYTE)0xE8;
			break;

		default:
			break;
	}

	*(DWORD*)((DWORD)pAddress + 1) = (DWORD)data - (DWORD)pAddress - 5;

	VirtualProtect(pAddress, 5, dwOldProtect, &dwOldProtect);
}
#endif