#include "StdInc.h"

static void __declspec(naked) PassNewArgument()
{
	__asm
	{
		mov [esi], ecx
		mov [esi + 4], edx

		mov ecx, [esp + 0Ch + 14h]
		mov [esi + 10h], ecx
		retn
	}
}

const DWORD MS_VC_EXCEPTION=0x406D1388;

#pragma pack(push,8)
typedef struct tagTHREADNAME_INFO
{
	DWORD dwType; // Must be 0x1000.
	LPCSTR szName; // Pointer to name (in user addr space).
	DWORD dwThreadID; // Thread ID (-1=caller thread).
	DWORD dwFlags; // Reserved for future use, must be zero.
} THREADNAME_INFO;
#pragma pack(pop)

void SetThreadName(DWORD dwThreadID, char* threadName)
{
	THREADNAME_INFO info;
	info.dwType = 0x1000;
	info.szName = threadName;
	info.dwThreadID = dwThreadID;
	info.dwFlags = 0;

	__try
	{
		RaiseException(MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(ULONG_PTR), (ULONG_PTR*)&info);
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{
	}
}

static LPTHREAD_START_ROUTINE origStart;

static DWORD WINAPI NewThreadStart(LPVOID arg)
{
	SetThreadName(GetCurrentThreadId(), *(((char**)arg) + 4));

	return origStart(arg);
}

static HookFunction hookFunction([] ()
{
	hook::call(0x5A8814, PassNewArgument);

	origStart = *(LPTHREAD_START_ROUTINE*)(0x5A882D);
	hook::put(0x5A882D, NewThreadStart);
});