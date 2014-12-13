/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <time.h>
#include <dbghelp.h>

LONG WINAPI CustomUnhandledExceptionFilter(LPEXCEPTION_POINTERS ExceptionInfo)
{
	// step 1: write minidump
	wchar_t error[1024];
	wchar_t filename[MAX_PATH];
	__time64_t time;
	tm* ltime;

	CreateDirectory(MakeRelativeCitPath(L"crashes").c_str(), nullptr);

	_time64(&time);
	ltime = _localtime64(&time);
	wcsftime(filename, _countof(filename) - 1, MakeRelativeCitPath(L"crashes\\crash-%Y%m%d%H%M%S.dmp").c_str(), ltime);
	_snwprintf(error, _countof(error) - 1, L"A minidump has been written to %s.", filename);

	HANDLE hFile = CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		MINIDUMP_EXCEPTION_INFORMATION ex;
		memset(&ex, 0, sizeof(ex));
		ex.ThreadId = GetCurrentThreadId();
		ex.ExceptionPointers = ExceptionInfo;
		ex.ClientPointers = FALSE;

		if (FAILED(MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hFile, MiniDumpNormal, &ex, NULL, NULL)))
		{
			_snwprintf(error, _countof(error) - 1, L"An error (0x%X) occurred during writing %s.", GetLastError(), filename);
		}

		CloseHandle(hFile);
	}
	else
	{
		_snwprintf(error, _countof(error) - 1, L"An error (0x%X) occurred during creating %s.", GetLastError(), filename);
	}

	// step 2: exit the application
	wchar_t errorCode[1024];
	wsprintf(errorCode, L"Fatal error (0x%08X) at 0x%08X.\n%s", ExceptionInfo->ExceptionRecord->ExceptionCode, ExceptionInfo->ExceptionRecord->ExceptionAddress, error);
	
	HWND wnd = nullptr;

#ifdef GTA_NY
	wnd = *(HWND*)0x1849DDC;
#endif

	MessageBox(wnd, errorCode, L"CitizenFX Fatal Error", MB_OK | MB_ICONSTOP);
	ExitProcess(1);

	return 0;
}