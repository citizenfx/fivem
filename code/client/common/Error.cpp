/*
 * This file is part of the CitizenFX project - http://citizen.re/
 *
 * See LICENSE and MENTIONS in the root of the source tree for information
 * regarding licensing.
 */

#include "StdInc.h"
#include <Error.h>

#if defined(COMPILING_LAUNCH) && !defined(LAUNCHER_PERSONALITY_MAIN) && !defined(COMPILING_DIAG)
#define NON_CRT_LAUNCHER
#endif

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE) && !defined(IS_FXSERVER)
#include <ICoreGameInit.h>
#endif

#include <fnv.h>

#if !defined(IS_FXSERVER)
#define RAPIDJSON_ASSERT(x) (void)0
#include <rapidjson/document.h>
#include <rapidjson/writer.h>

template<typename T>
static std::string FormatErrorPickup(std::string_view buffer, const T& thisError)
{
	rapidjson::Document document;
	document.SetObject();
	document.AddMember("message", rapidjson::Value(buffer.data(), rapidjson::SizeType(buffer.length())), document.GetAllocator());
	document.AddMember("file", rapidjson::Value(std::get<0>(thisError).data(), rapidjson::SizeType(std::get<0>(thisError).length())), document.GetAllocator());
	document.AddMember("line", rapidjson::Value(std::get<1>(thisError)), document.GetAllocator());
	document.AddMember("sigHash", rapidjson::Value(std::get<2>(thisError)), document.GetAllocator());

	rapidjson::StringBuffer sbuffer;
	rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);

	document.Accept(writer);

	return std::string{ sbuffer.GetString(), sbuffer.GetSize() };
}
#endif

#include <CrossBuildRuntime.h>

#define ERR_NORMAL 0 // will continue game execution, but not here
#define ERR_FATAL 1

#define BUFFER_LENGTH 32768

#ifdef _WIN32
#include <wtsapi32.h>
#else
#include <signal.h>
#endif

#if defined(ERROR_CRASH_MAGIC) && defined(_WIN32)
bool IsErrorException(PEXCEPTION_POINTERS ep)
{
	return (ep->ExceptionRecord->ExceptionCode == EXCEPTION_ACCESS_VIOLATION && ep->ExceptionRecord->ExceptionInformation[1] == ERROR_CRASH_MAGIC);
}
#endif

static thread_local std::tuple<std::string_view, int, uint32_t> g_thisError;

static bool IsUserConnected()
{
#ifdef _WIN32
	auto wtsapi = LoadLibraryW(L"wtsapi32.dll");

	if (wtsapi)
	{
		auto _WTSQuerySessionInformationW = (decltype(&WTSQuerySessionInformationW))GetProcAddress(wtsapi, "WTSQuerySessionInformationW");
		auto _WTSFreeMemory = (decltype(&WTSFreeMemory))GetProcAddress(wtsapi, "WTSFreeMemory");

		if (_WTSQuerySessionInformationW)
		{
			LPWSTR data;
			DWORD dataSize;
			if (_WTSQuerySessionInformationW(WTS_CURRENT_SERVER_HANDLE, WTS_CURRENT_SESSION, WTSConnectState, &data, &dataSize))
			{
				auto connectState = *(WTS_CONNECTSTATE_CLASS*)data;
				
				bool rv = (connectState == WTSActive);

				_WTSFreeMemory(data);

				return rv;
			}
		}
	}
#endif

	return true;
}

#if defined(COMPILING_LAUNCH) && !defined(COMPILING_DIAG)
extern bool MinidumpInitialized();
#endif

static int SysError(const char* buffer)
{
#ifdef WIN32
	HWND wnd = CoreGetGameWindow();

#if !defined(COMPILING_LAUNCH) && !defined(COMPILING_CONSOLE)
	if (CoreIsDebuggerPresent())
#else
	if (IsDebuggerPresent())
#endif
	{
		__debugbreak();
	}

#if !defined(COMPILING_CONSOLE) && !defined(IS_FXSERVER)
	auto errorPickup = FormatErrorPickup(buffer, g_thisError);
	FILE* f = _wfopen(MakeRelativeCitPath(L"data\\cache\\error-pickup").c_str(), L"wb");

	if (f)
	{
		fprintf(f, "%s", errorPickup.c_str());
		fclose(f);

#if defined(COMPILING_LAUNCH) && !defined(COMPILING_DIAG)
		if (MinidumpInitialized())
#endif
		{
			return -1;
		}
	}
#endif

#if defined(IS_FXSERVER)
	fprintf(stderr, "Fatal Error: %s", buffer);
#endif

	if (IsUserConnected())
	{
		MessageBoxW(wnd, ToWide(buffer).c_str(), L"Fatal Error", MB_OK | MB_ICONSTOP);
	}

#ifdef _DEBUG
#ifndef IS_FXSERVER
	assert(!"breakpoint time");
#endif
#endif

	TerminateProcess(GetCurrentProcess(), 1);
#else
	fprintf(stderr, "%s", buffer);

	raise(SIGTERM);
#endif

	return 0;
}

struct ErrorDataPerProcess
{
	bool inFatalError = false;
	std::string lastFatalError;
};

struct ErrorData
{
	ErrorDataPerProcess* perProcess;

	bool inRecursiveError = false;
	std::string lastRecursiveError;

	bool inError = false;
	std::string lastError;
};

#if defined(COMPILING_CORE) || defined(COMPILING_LAUNCH)
extern "C" DLL_EXPORT ErrorData* GetErrorData()
{
	static thread_local ErrorData errorData;
	if (!errorData.perProcess)
	{
		static ErrorDataPerProcess edpp;
		errorData.perProcess = &edpp;
	}

	return &errorData;
}
#elif defined(_WIN32)
inline ErrorData* GetErrorData()
{
	using TCoreFunc = decltype(&GetErrorData);

	static TCoreFunc func;

	if (!func)
	{
		auto hCore = GetModuleHandleW(L"CoreRT.dll");

		if (hCore)
		{
			func = (TCoreFunc)GetProcAddress(hCore, "GetErrorData");
		}
	}

	return (func) ? func() : 0;
}
#else
extern "C" ErrorData* GetErrorData();
#endif

static int GlobalErrorHandler(int eType, const char* buffer)
{
	auto errorData = GetErrorData();

	if (!errorData)
	{
		static thread_local ErrorData dummyErrorData;
		if (!dummyErrorData.perProcess)
		{
			static ErrorDataPerProcess edpp;
			dummyErrorData.perProcess = &edpp;
		}

		errorData = &dummyErrorData;
	}

	bool& inError = errorData->inError;
	std::string& lastError = errorData->lastError;
	bool& inFatalError = errorData->perProcess->inFatalError;
	std::string& lastFatalError = errorData->perProcess->lastFatalError;

	trace("Error: %s\n", buffer);

	if (inError || (eType == ERR_FATAL && inFatalError))
	{
		bool& inRecursiveError = errorData->inRecursiveError;
		std::string& lastRecursiveError = errorData->lastRecursiveError;

		if (inRecursiveError)
		{
			return SysError(va("Recursive-recursive error: %s\n%s", buffer, lastRecursiveError));
		}

		auto e = va("Recursive error: %s\nOriginal error: %s",
			buffer,
			lastFatalError.empty()
				? lastError
				: lastFatalError);

		inRecursiveError = true;
		lastRecursiveError = e;
		return SysError(e);
	}

	inError = true;
	lastError = buffer;

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
		inFatalError = true;
		lastFatalError = buffer;

		return SysError(buffer);
	}

	inError = false;

	return 0;
}

struct ScopedError
{
	ScopedError(std::string_view file, int line, uint32_t stringHash)
	{
		g_thisError = std::make_tuple(file, line, stringHash);
	}

	~ScopedError()
	{
		g_thisError = std::make_tuple(std::string_view{}, 0, 0);
	}
};

#if defined(COMPILING_LAUNCHER)
void FatalErrorV(const char* string, fmt::printf_args formatList)
{
	GlobalErrorHandler(ERR_FATAL, fmt::vsprintf(string, formatList).c_str());
}
#endif

#if (!defined(COMPILING_DIAG) && !defined(COMPILING_CONSOLE) && !defined(COMPILING_SHARED_LIBC)) || defined(NON_CRT_LAUNCHER)
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

int FatalErrorNoExceptRealV(const char* file, int line, uint32_t stringHash, const char* string, fmt::printf_args formatList)
{
#if !defined(IS_FXSERVER) && !defined(COMPILING_LAUNCH)
	FatalErrorRealV(file, line, stringHash, string, formatList);
	return -1;
#endif

	return FatalErrorRealV(file, line, stringHash, string, formatList);
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

#if (defined(COMPILING_LAUNCH) || defined(COMPILING_CONSOLE) || defined(COMPILING_SHARED_LIBC)) && !defined(NON_CRT_LAUNCHER)
#undef _wassert
#undef NDEBUG

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
