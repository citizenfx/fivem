/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
//#include "GameInit.h"
#include <ICoreGameInit.h>

#define ERR_NORMAL 0 // will continue game execution, but not here
#define ERR_FATAL 1

#define BUFFER_LENGTH 32768

static int SysError(const char* buffer)
{
#ifdef WIN32
	HWND wnd = FindWindow(L"grcWindow", nullptr);

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
	if (CoreIsDebuggerPresent())
#else
	if (IsDebuggerPresent())
#endif
	{
		__debugbreak();
	}

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\error-pickup").c_str(), L"wb");

	if (f)
	{
		fprintf(f, "%s", buffer);
		fclose(f);

#ifdef _DEBUG
		assert(!"breakpoint time");
#endif

		return -1;
	}
#endif

	MessageBoxA(wnd, buffer, "Fatal Error", MB_OK | MB_ICONSTOP);

#ifdef _DEBUG
	assert(!"breakpoint time");
#endif

	TerminateProcess(GetCurrentProcess(), 1);
#else
	fprintf(stderr, "%s", buffer);

	abort();
#endif

	return 0;
}

static int GlobalErrorHandler(int eType, const char* buffer)
{
	static thread_local bool inError = false;

	trace("GlobalError: %s\n", buffer);

	if (inError)
	{
		static thread_local bool inRecursiveError = false;

		if (inRecursiveError)
		{
			return SysError("RECURSIVE RECURSIVE ERROR");
		}

		inRecursiveError = true;
		return SysError(va("Recursive error: %s", buffer));
	}

	inError = true;

	if (eType == ERR_NORMAL)
	{
#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
		ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();
		bool handled = false;

		if (gameInit && gameInit->TriggerError(buffer))
		{
			handled = true;
		}
		
		if (gameInit && gameInit->GetGameLoaded())
		{
			static wchar_t wbuffer[BUFFER_LENGTH];
			mbstowcs(wbuffer, buffer, _countof(wbuffer));

			gameInit->KillNetwork(wbuffer);

			handled = true;
		}
		
		if (!handled)
#endif
		{
			return SysError(buffer);
		}
	}
	else
	{
		return SysError(buffer);
	}

	inError = false;

	return 0;
}

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(COMPILING_SHARED_LIBC)
int GlobalErrorReal(const char* string, const fmt::ArgList& formatList)
{
	return GlobalErrorHandler(ERR_NORMAL, fmt::sprintf(string, formatList).c_str());
}

int FatalErrorReal(const char* string, const fmt::ArgList& formatList)
{
	return GlobalErrorHandler(ERR_FATAL, fmt::sprintf(string, formatList).c_str());
}
#else
void GlobalError(const char* string, const fmt::ArgList& formatList)
{
	GlobalErrorHandler(ERR_NORMAL, fmt::sprintf(string, formatList).c_str());
}

void FatalError(const char* string, const fmt::ArgList& formatList)
{
	GlobalErrorHandler(ERR_FATAL, fmt::sprintf(string, formatList).c_str());
}
#endif