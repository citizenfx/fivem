/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
//#include "GameInit.h"

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(IS_FXSERVER)
#include <ICoreGameInit.h>
#endif

#include <fnv.h>

#include <json.hpp>

using json = nlohmann::json;

#define ERR_NORMAL 0 // will continue game execution, but not here
#define ERR_FATAL 1

#define BUFFER_LENGTH 32768

static thread_local std::tuple<const char*, int, uint32_t> g_thisError;

static int SysError(const char* buffer)
{
#ifdef WIN32
	HWND wnd = FindWindow(
#ifdef GTA_FIVE
		L"grcWindow"
#elif defined(IS_RDR3)
		L"sgaWindow"
#else
		L"UNKNOWN_WINDOW"
#endif
	, nullptr);

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
	if (CoreIsDebuggerPresent())
#else
	if (IsDebuggerPresent())
#endif
	{
		__debugbreak();
	}

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(IS_FXSERVER)
	json o = json::object();
	o["message"] = buffer;
	o["file"] = std::get<0>(g_thisError);
	o["line"] = std::get<1>(g_thisError);
	o["sigHash"] = std::get<2>(g_thisError);

	FILE* f = _wfopen(MakeRelativeCitPath(L"cache\\error-pickup").c_str(), L"wb");

	if (f)
	{
		fprintf(f, "%s", o.dump().c_str());
		fclose(f);

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
			return SysError(va("Recursive-recursive error: %s", buffer));
		}

		inRecursiveError = true;
		return SysError(va("Recursive error: %s", buffer));
	}

	inError = true;

	if (eType == ERR_NORMAL)
	{
#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(IS_FXSERVER)
		ICoreGameInit* gameInit = Instance<ICoreGameInit>::Get();
		bool handled = false;

		if (gameInit && gameInit->TriggerError(buffer))
		{
			handled = true;
		}
		
		if (gameInit && gameInit->GetGameLoaded())
		{
			gameInit->KillNetwork(ToWide(buffer).c_str());

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

struct ScopedError
{
	ScopedError(const char* file, int line, uint32_t stringHash)
	{
		g_thisError = std::make_tuple(file, line, stringHash);
	}

	~ScopedError()
	{
		g_thisError = std::make_tuple(nullptr, 0, 0);
	}
};

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(COMPILING_SHARED_LIBC)
int GlobalErrorRealV(const char* file, int line, uint32_t stringHash, const char* string, fmt::printf_args formatList)
{
	ScopedError error(file, line, stringHash);
	return GlobalErrorHandler(ERR_NORMAL, fmt::vsprintf(string, formatList).c_str());
}

int FatalErrorRealV(const char* file, int line, uint32_t stringHash, const char* string, fmt::printf_args formatList)
{
	ScopedError error(file, line, stringHash);
	return GlobalErrorHandler(ERR_FATAL, fmt::vsprintf(string, formatList).c_str());
}
#else
void GlobalErrorV(const char* string, fmt::printf_args formatList)
{
	GlobalErrorHandler(ERR_NORMAL, fmt::vsprintf(string, formatList).c_str());
}

void FatalErrorV(const char* string, fmt::printf_args formatList)
{
	GlobalErrorHandler(ERR_FATAL, fmt::vsprintf(string, formatList).c_str());
}
#endif

#if defined(COMPILING_LAUNCH) || defined(COMPILING_CONSOLE) || defined(COMPILING_SHARED_LIBC)
#undef _wassert

#include <assert.h>

void __cdecl _wwassert(
	_In_z_ wchar_t const* _Message,
	_In_z_ wchar_t const* _File,
	_In_   unsigned       _Line
)
{
	_wassert(_Message, _File, _Line);
}
#endif
