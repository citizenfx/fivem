// hooks to prevent SecuROM from messing up our activities

#include "StdInc.h"

// DFA stuff

HANDLE CreateFileHook(_In_ LPCSTR lpFileName, _In_ DWORD dwDesiredAccess, _In_ DWORD dwShareMode, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes, _In_ DWORD dwCreationDisposition, _In_ DWORD dwFlagsAndAttributes, _In_opt_ HANDLE hTemplateFile)
{
	HANDLE hFile = CreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);

	return hFile;
}

BOOL CloseHandleHook(_In_ HANDLE hObject)
{
	__try
	{
		BOOL retval = CloseHandle(hObject);

		return retval;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
	{

	}

	return TRUE;
}

BOOL GetFileSizeExHook(HANDLE hFile, PLARGE_INTEGER lpFileSize)
{
	return GetFileSizeEx(hFile, lpFileSize);
}

BOOL ReadFileHook(HANDLE hFile, LPVOID pBuffer, DWORD nNumberOfBytesRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped)
{
	return ReadFile(hFile, pBuffer, nNumberOfBytesRead, lpNumberOfBytesRead, lpOverlapped);
}

DWORD SetFilePointerHook(HANDLE hFile, LONG dtm, PLONG dtmHigh, DWORD mm)
{
	return SetFilePointer(hFile, dtm, dtmHigh, mm);
}

DEFINE_INJECT_HOOK(createFileHook, 0xD2F994)
{
	return JumpTo((DWORD)CreateFileHook);
}

DEFINE_INJECT_HOOK(closeHandleHook, 0xD2FB2C)
{
	return JumpTo((DWORD)CloseHandleHook);
}

DEFINE_INJECT_HOOK(setFilePointerHook, 0xD2FB53)
{
	return JumpTo((DWORD)SetFilePointerHook);
}

DEFINE_INJECT_HOOK(getFileSizeExHook, 0xD2FBA2)
{
	return JumpTo((DWORD)GetFileSizeExHook);
}

DEFINE_INJECT_HOOK(readFileHook, 0xD2F9F9)
{
	return JumpTo((DWORD)ReadFileHook);
}

DEFINE_INJECT_HOOK(dfaJumpHook1, 0x5AAF57)
{
	return JumpTo(0x5AB077);
}

DEFINE_INJECT_HOOK(dfaJumpHook2, 0x5AB11B)
{
	return JumpTo(0x5AB23B);
}

static HookFunction hookFunction([] ()
{
	// DFA init call (needs to return 1 as int or a RMN60 error will be shown)
	static hook::inject_call<int, int> dfaInit(0x5AAC6D);
	dfaInit.inject([] (int)
	{
		return 1;
	});

	// DFA call hooks
	createFileHook.inject();
	closeHandleHook.inject();
	setFilePointerHook.inject();
	getFileSizeExHook.inject();
	readFileHook.inject();
	dfaJumpHook1.inject();
	dfaJumpHook2.inject();
});