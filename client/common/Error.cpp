#include "StdInc.h"
#ifndef COMPILING_LAUNCH
#include "GameInit.h"
#endif

#define ERR_NORMAL 0 // will continue game execution, but not here
#define ERR_FATAL 1

#define BUFFER_LENGTH 32768

static void SysError(const char* buffer)
{
	HWND wnd = nullptr;

#ifdef GTA_NY
	wnd = *(HWND*)0x1849DDC;
#endif

#ifdef WIN32
#ifdef _M_IX86
#ifdef _DEBUG
	if (IsDebuggerPresent())
	{
		__asm int 3
	}
#endif
#endif
#endif

	MessageBoxA(wnd, buffer, "CitizenFX Fatal Error", MB_OK | MB_ICONSTOP);
	ExitProcess(1);
}

static void GlobalErrorHandler(int eType, const char* buffer)
{
	static bool inError = false;

	trace("GlobalError: %s\n", buffer);

	if (inError)
	{
		static bool inRecursiveError = false;

		if (inRecursiveError)
		{
			SysError("RECURSIVE RECURSIVE ERROR");
		}

		inRecursiveError = true;
		SysError(va("Recursive error: %s", buffer));
	}

	inError = true;

	if (eType == ERR_NORMAL)
	{
#ifndef COMPILING_LAUNCH
		// TODO: UI killer for pre-connected state
		if (GameInit::GetGameLoaded())
		{
			static wchar_t wbuffer[BUFFER_LENGTH];
			mbstowcs(wbuffer, buffer, _countof(wbuffer));

			GameInit::KillNetwork(wbuffer);
		}
		else
#endif
		{
			SysError(buffer);
		}
	}
	else
	{
		SysError(buffer);
	}

	inError = false;
}

void GlobalError(const char* string, ...)
{
	static char buffer[BUFFER_LENGTH];

	va_list ap;
	va_start(ap, string);
	int length = _vsnprintf_s(buffer, BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		SysError("Attempted to overrun string in call to GlobalError()!");
	}

	GlobalErrorHandler(ERR_NORMAL, buffer);
}

void FatalError(const char* string, ...)
{
	static char buffer[BUFFER_LENGTH];

	va_list ap;
	va_start(ap, string);
	int length = _vsnprintf_s(buffer, BUFFER_LENGTH, string, ap);
	va_end(ap);

	if (length >= BUFFER_LENGTH)
	{
		SysError("Attempted to overrun string in call to FatalError()!");
	}

	GlobalErrorHandler(ERR_FATAL, buffer);
}
